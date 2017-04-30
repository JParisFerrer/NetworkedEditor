#include <curses.h>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <signal.h>
#include <vector>
#include <cstdio>
#include "textcontainer.h"
#include "blockingVector.h"
#include "lockfreelist.h"

//using namespace std;
#include "editor.h"

extern std::string SERVER_SEARCH_PORT;
extern std::string SERVER_ADDRESS;
extern bool START_SERVER;

extern volatile bool SHUTDOWN_NETWORK;

namespace client
{

#define CTRL_W 23
    // the KEY_ENTER is wrong, use this constant instead
#define ENTER_KEY 13
#define CTRL_Q 17
#define DELETE_KEY 330
#define MAC_BACK 127

    WINDOW* mainWindow;
    WINDOW* commandWindow;
    WINDOW* currWindow;

    
    int SERVER_SOCKET;

    volatile bool SERVER_LOST = false;
    volatile bool HANDLED_SERVER_LOST = false;

    //vector<vector< int >> data;
    TextContainer<BlockingVector> text;
    //TextContainer<LockFreeList> text;
    std::vector<int> commands;

    std::mutex mlock;
    std::mutex glock;

    std::thread thread_messageHandler;
    std::thread thread_getFull;

    ssize_t numdisplaylines = 1;
    ssize_t numlines = 1;
    ssize_t lineoffset = 0;

    // https://stackoverflow.com/a/7408245
    std::vector<std::string> split(const std::string &text, char sep)
    {
        std::vector<std::string> tokens;
        std::size_t start = 0, end = 0;
        while ((end = text.find(sep, start)) != std::string::npos)
        {
            if (end != start)
                tokens.push_back(text.substr(start, end - start));
            start = end + 1;
        }
        if (end != start)
            tokens.push_back(text.substr(start));
        return tokens;
    }

    void move_cursor(size_t y, size_t x)
    {
        text.move(y, x);
        if(!SERVER_LOST && !SHUTDOWN_NETWORK)
            send_move(SERVER_SOCKET, y, x);
    }

    void insert_char(size_t y, size_t x, int c)
    {
        text.insert(y, x, c);
        if(!SERVER_LOST && !SHUTDOWN_NETWORK)
            send_insert(SERVER_SOCKET, y, x, c);
    }

    void remove_char(size_t y, size_t x)
    {
        text.remove(y, x);
        if(!SERVER_LOST && !SHUTDOWN_NETWORK)
            send_remove(SERVER_SOCKET, y, x);
    }

    void reset_x(WINDOW* win, bool notify = true)
    {
        int x, y;
        getyx(win, y, x);
        wmove(win, y, 0);

        if(win == mainWindow && notify)
            move_cursor(y + lineoffset, 0);
            //text.move(y + lineoffset, 0);
    }

    void clear_cmd_window(bool clear_commands = true)
    {
        if(clear_commands)
        {
            for(auto& c : commands)
                c = ' ';
        }

        // don't notify because this is command window stuff
        reset_x(commandWindow, false);

        waddstr(commandWindow, "                                                                                                                                                                                                                  ");
        // don't notify because this is command window stuff
        reset_x(commandWindow, false);

        wrefresh(commandWindow);
    }

    void print_in_cmd_window(const char* c, int sleeptime = 1)
    {
        clear_cmd_window();
        reset_x(commandWindow, false);
        waddstr(commandWindow, c);
        wrefresh(commandWindow);
        sleep(sleeptime);
        clear_cmd_window();
        wrefresh(commandWindow);
    }

    void put_in_cmd_window(const char* c)
    {
        clear_cmd_window();
        reset_x(commandWindow, false);
        waddstr(commandWindow, c);
        wrefresh(commandWindow);

    }

    void refresh_screen()
    {
        //log("in function %s", __func__);
        int sx_main, sy_main, sx_command, sy_command;
        int maxx, maxy;
        getmaxyx(stdscr, maxy, maxx);

        getyx(mainWindow, sy_main, sx_main);
        getyx(commandWindow, sy_command, sx_command);

        //for (size_t y = 0; y < numdisplaylines; y++)
        //{
        //    text.print(lineoffset + y, maxx);
        //}

        text.print(mainWindow, lineoffset, maxx);

        for(int x = 0; x < commands.size(); x++)
        {
            // 0 is y coord in the window
            mvwaddch(commandWindow, 0, x, commands[x]);
        }

        wmove(mainWindow, sy_main, sx_main);
        wmove(commandWindow, sy_command, sx_command);

        wrefresh(mainWindow);
        //wrefresh(commandWindow);
        wrefresh(currWindow);       // the cursor is drawn on refresh

        //log("left function %s", __func__);
    }

    void move_win_rel(WINDOW* win, int xoffs, int yoffs)
    {
        //log("in function %s", __func__);

        int x, y;
        getyx(win, y, x);

        if(win == mainWindow)
        {
            int capped_x, capped_y;
            capped_y = y + yoffs;

            if(capped_y > numdisplaylines-1)
            {
                // we went over the edge, try to scroll
                if(lineoffset + capped_y < numlines)
                    lineoffset += capped_y - numdisplaylines + 1;

                capped_y = numdisplaylines-1;
            }

            if(capped_y < 0)
            {
                // we went over the edge, try to scroll
                if(lineoffset > 0 && abs(capped_y) <= lineoffset)
                {
                    lineoffset -= abs(capped_y);
                }

                capped_y = 0;
            }

            capped_x = std::min((ssize_t)(x + xoffs), (ssize_t)text.line_width(capped_y));

            if(capped_x < 0)
                capped_x = 0;

            wmove(win, capped_y, capped_x);
            //text.move(capped_y + lineoffset, capped_x);
            move_cursor(capped_y + lineoffset, capped_x);
        }
        else
        {
            // command window
            int capped_x = std::min(x + xoffs, (int)commands.size());

            wmove(win, 0, capped_x);
        }

        //log("left function %s", __func__);

    }

    void exit_handler(int sigint)
    {
        log("in function %s", __func__);

        endwin();

        // tell those threads to die
        SERVER_LOST = true;
        SHUTDOWN_NETWORK = true;

        // force that persistent thread to die
        close(SERVER_SOCKET);

        thread_messageHandler.join();
        thread_getFull.join();


        exit(1);
    }

    void resize_handler(int sigwinch)
    {
        log("in function %s", __func__);

        endwin();
        refresh();
        clear();

        int  w, h;
        getmaxyx(stdscr, h, w);

        wresize(mainWindow, h - 1, w);
        wresize(commandWindow, 1, w);
        mvwin(commandWindow, h-1, 0);

        if(h-1 < numdisplaylines)
        {
            numdisplaylines = h-1;
        }

        /*
           data.resize(h-1);
           for(auto& v : data)
           v.resize(w, ' ');
           */
        if(w > 0)
            commands.resize(w, ' ');

        //refresh_screen();

        log("left function %s", __func__);
    }

    // credit to https://beej.us/guide/bgnet/output/html/multipage/clientserver.html
    int network_setup()
    {
        log("in function %s", __func__);

        SHUTDOWN_NETWORK = false;

        struct addrinfo hints, *server_info, *traverser;
        int yes = 1, no = 0;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        int ret = getaddrinfo(SERVER_ADDRESS.c_str(), SERVER_SEARCH_PORT.c_str(), &hints, &server_info);
        if(ret)
        {
            // would use cerr but formatting is nice
            log("getaddrinfo failed: %s", gai_strerror(ret));
            return 1;
        }

        int socket_fd;
        // let's do this properly and loop through, choosing the first working method
        for(traverser = server_info; traverser != nullptr; traverser = traverser->ai_next) 
        {
            if ((socket_fd = socket(traverser->ai_family, traverser->ai_socktype, traverser->ai_protocol)) == -1) 
            {
                perror("client: socket");
                continue;
            }

            if (setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY, (void*)&no, sizeof(int)) == -1)
            {
                perror("setsockopt");
                return 5;

            }
            if (connect(socket_fd, traverser->ai_addr, traverser->ai_addrlen) == -1) 
            {
                close(socket_fd);
                perror("client: connect");
                continue;
            }

            log("Connected to server at %s on port %s", SERVER_ADDRESS.c_str(), SERVER_SEARCH_PORT.c_str());

            // if we made it here, then we connected fine
            break;
        }

        if (traverser == nullptr)
        {
            log("Failed to connect to '%s' using port '%s'", SERVER_ADDRESS.c_str(), SERVER_SEARCH_PORT.c_str());
            return 2;
        }

        freeaddrinfo(server_info);

        SERVER_SOCKET = socket_fd;

        return 0;
    }

    int setup()
    {
        log("in function %s", __func__);

        int ret = network_setup();

        log("Finished client network setup");

        if(ret)
        {
            log("Error in networking setup! code: %d", ret);
            return ret;
        }

        initscr();      // Init the library
        //cbreak();       // set it up so we read a character at a time
        raw();          // set it up so we read a character at a time
        halfdelay(1);   // get right away, but leave function every 1/10th of a second
        nonl();
        noecho();       // prevent the screen from showing typed in characters

        use_extended_names(TRUE);

        struct sigaction res;
        res.sa_handler = resize_handler;
        sigfillset(&res.sa_mask);
        res.sa_flags = 0 & SA_RESTART;
        sigaction(SIGWINCH, &res, NULL);
        //signal(SIGWINCH, resize_handler);
        //signal(SIGINT, exit_handler);

        wrefresh(stdscr);

        int w, h;
        getmaxyx(stdscr, h, w);

        //w = 80;
        //h = 10;

        mainWindow = newwin(h - 1, w, 0, 0);
        commandWindow = newwin(1, w, h-1, 0);

        /*
           data.resize(h-1);
           for(auto & v : data)
           v.resize(w, ' ');
           */
        commands.resize(w, ' ');

        nodelay(mainWindow, FALSE);
        nodelay(commandWindow, FALSE);


        wrefresh(mainWindow);
        wrefresh(commandWindow);


        keypad(mainWindow, TRUE); // we should handle the special chars ourselves
        keypad(commandWindow, TRUE);
        keypad(stdscr, TRUE);

        if(strcmp(SERVER_ADDRESS.c_str(), "127.0.0.1") != 0)
        {
            char* c;
            asprintf(&c, "Connected to %s", SERVER_ADDRESS.c_str());
            print_in_cmd_window(c);
            free(c);
        }

        log("Finished setting up client");

        return 0;
    }

    void signal_disconnect()
    {
        SERVER_LOST = true;
        SHUTDOWN_NETWORK = true;

        log("Client lost connection to the server");
    }

    void handle_disconnect()
    {
        int c;
        const char* msg = "Disconnected! (s)ave locally and quit, (c)ontinue locally, or (q)uit?";

        nocbreak();
        raw();

        while(1)
        {
            put_in_cmd_window(msg);

            c = wgetch(commandWindow);

            if(c == 's')
            {
                log("client chose to save locally and quit");

                clear_cmd_window();

                print_in_cmd_window("enter file name to save into:", 2);

                // move to the beginning of the window
                wmove(commandWindow, 0, 0);

                while(1)
                {
                    c = wgetch(commandWindow);

                    //log("client got %d (%c)", c, (char)c);

                    if (c == KEY_UP)
                    {
                        move_win_rel(commandWindow, 0, -1);
                    }
                    else if (c == KEY_DOWN)
                    {
                        move_win_rel(commandWindow, 0, 1);
                    }
                    else if (c == KEY_LEFT)
                    {
                        move_win_rel(commandWindow, -1, 0);
                    }
                    else if (c == KEY_RIGHT)
                    {
                        move_win_rel(commandWindow, 1, 0);
                    }
                    else if(c == ENTER_KEY || c == 10)
                    {
                        std::stringstream str;
                        for(auto it = commands.begin(); it != commands.end(); it++)
                            str << (char)*it;

                        //print_in_cmd_window(str.str().c_str());

                        std::vector<std::string> v = split(str.str(), ' ');
                        log("v size = %lu, ,v[0]=%s", v.size(), v[0].c_str());
                        v.pop_back();

                        if(v.size() == 1)
                        {
                            text.writeToFile(v[0]);
                            char* c;

                            asprintf(&c, "Saved file: %s", v[0].c_str());
                            print_in_cmd_window(c);
                            free(c);        // I think it uses malloc

                            //log("Writing file %s", v[1].c_str());

                            //send_write(SERVER_SOCKET, v[1]);

                            endwin();
                            log("Client chose to save and exit after disconnect");

                            // force that persistent thread to die
                            close(SERVER_SOCKET);

                            thread_messageHandler.join();
                            thread_getFull.join();

                            exit(0);
                        }
                        else
                        {
                            // bad args
                            print_in_cmd_window("Bad # of args: ");
                            print_in_cmd_window(std::to_string(v.size()).c_str());

                            print_in_cmd_window("enter file name to save into:", 2);
                        }
                    }
                    else if (c == KEY_BACKSPACE)
                    {
                        int x, y;
                        getyx(currWindow, y, x);

                        if(x > 0)
                        {
                            move_win_rel(commandWindow, -1, 0);

                            getyx(commandWindow, y, x);

                            //commands[x] = ' ';
                            commands.erase(commands.begin() + x);
                            commands.push_back(' ');

                        }
                    }
                    else if (c == DELETE_KEY)
                    {
                        // like backspace but the other way
                        int x, y;
                        getyx(currWindow, y, x);

                        //commands[x] = ' ';
                        commands.erase(commands.begin() + x);
                        commands.push_back(' ');
                    }
                    else if (isprint(c))
                    {
                        int x, y;
                        getyx(commandWindow, y, x);

                        move_win_rel(commandWindow, 1, 0);

                        //mvwaddch(commandWindow, y, x, c);
                        commands[x] = c;

                    }
                    else {} // garbage char

                    int y, x;
                    getyx(commandWindow, y, x);

                    //log("updating command window");

                    for(int xi = 0; xi < commands.size(); xi++)
                    {
                        // 0 is y coord in the window
                        mvwaddch(commandWindow, 0, xi, commands[xi]);
                        //log("%c", (char)commands[xi]);
                    }
                    //log("");

                    wmove(commandWindow, y, x);

                    wrefresh(commandWindow);
                }
            } 
            else if (c == 'c')
            {
                log("Client chose to continue locally after disconnect");

                HANDLED_SERVER_LOST = true;

                clear_cmd_window();

                break;
            }
            else if (c == 'q')
            {
                endwin();

                log("Client chose to exit after disconnect");

                // force that persistent thread to die
                close(SERVER_SOCKET);

                thread_messageHandler.join();
                thread_getFull.join();

                exit(0);
            }
            else 
            {
                // invalid option, go again

                print_in_cmd_window("invalid option! please choose s, c, or q");
            }
        }

        halfdelay(1);

    }

    void handleMessage(std::pair<char*, size_t> msg)
    {
        PacketType type = get_bytes_as<PacketType>(msg.first, 0);

        //log("Client got message of type %d, %s", (short)type, ((short)type < PacketTypeNum ? PacketTypeNames[(short)type].c_str() : ""));

        switch(type)
        {
            case PacketType::WriteConfirmed:
                {
                    // just a string
                    std::string filename(msg.first + sizeof(short));

                    char* c;

                    asprintf(&c, "Saved file: %s", filename.c_str());
                    print_in_cmd_window(c);
                    free(c);        // I think it uses malloc

                    clear_cmd_window();

                    break;
                }

            case PacketType::ReadConfirmed:
                {
                    // a number of lines and then a string
                    size_t linesread = get_bytes_as<size_t>(msg.first, sizeof(short));

                    std::string filename(msg.first + sizeof(short) + sizeof(size_t));

                    char* c;

                    asprintf(&c, "Read file: %s [%lu lines]", filename.c_str(), linesread);
                    print_in_cmd_window(c);
                    free(c);        // I think it uses malloc

                    clear_cmd_window();

                    //send_get_full(SERVER_SOCKET);

                    break;
                }

            case PacketType::FullContent:
                {
                    // rest of it is serialized thing
                    log("got full content");

                    size_t lines = text.deserialize(msg.first + sizeof(short), msg.second - sizeof(short));

                    numlines = lines;

                    int y, x;
                    getmaxyx(mainWindow, y, x);

                    if(numlines > y)
                        numdisplaylines = y;
                    else
                        numdisplaylines = numlines;

                    //wrefresh(mainWindow);
                    glock.unlock();
                    log("unlocked glock");

                    break;
                }

            case PacketType::Insert:
                {
                    //fprintf(stdout, "Got insert!");

                    size_t y, x;
                    int c;

                    y = get_bytes_as<size_t>(msg.first, sizeof(short));
                    x = get_bytes_as<size_t>(msg.first, sizeof(short) + sizeof(size_t));
                    c = get_bytes_as<int>(msg.first, sizeof(short) + sizeof(size_t) * 2);

                    text.insert(y, x, c);

                    if(c == ENTER_KEY)
                    {
                        int x, y;
                        int maxx, maxy;
                        getmaxyx(mainWindow, maxy, maxx);
                        getyx(mainWindow, y, x);
                        numlines++;
                        if(numdisplaylines < maxy)
                            numdisplaylines++;

                        //text.insert(y + lineoffset, x, in);
                    }



                    break;
                }

            case PacketType::Remove:
                {
                    //fprintf(stdout, "Got remove!");

                    size_t y, x;
                    y = get_bytes_as<size_t>(msg.first, sizeof(short));
                    x = get_bytes_as<size_t>(msg.first, sizeof(short) + sizeof(size_t));

                    // x == 0
                    if(x == -1)
                    {
                           // only subtract numdisplaylines if we are running out of lines
                            if(numlines == numdisplaylines)
                                numdisplaylines--;
                            numlines--;

                            //move_win_rel(mainWindow, above_width, -1);
                    }
                    text.remove(y, x);

                    break;
                }

            default:
                {
                    log("Client got unhandled message type %d = %s", (short)type, ((short)type < PacketTypeNum ? PacketTypeNames[(short)type].c_str() : ""));
                    break;
                }
        }

        mlock.unlock();
    }

    void handleMessages()
    {
        //log("GOT HERE");

        while(1)
        {
            //log("Client getting message");
            std::pair<char*, size_t> msg = get_message(SERVER_SOCKET, true);
            //log("Client got message");

            if(SERVER_LOST || SHUTDOWN_NETWORK)
                return;

            if(msg.first)
            {

                mlock.lock();

                //log("Client got lock");

                handleMessage(msg);

                //log("Client got message");

                free_message(msg.first);

                // moved to inside handleMessage so that we can choose not to unlock it (for full reqs)
                //mlock.unlock();
            }
            else if (msg.second == -1)
            {
                // they disconnected
                signal_disconnect();

                // leave the thread
                return;
            }
        }
    }

    void getFullLoop()
    {
        while(!SERVER_LOST && !SHUTDOWN_NETWORK)
        {
            sleep(3);
            log("locking glock");
            glock.lock();
            log("locked glock");
            send_get_full(SERVER_SOCKET);
        }
    }

    int client_entrypoint()
    {
        log("in function %s", __func__);

        sleep(1);
        int ret = setup();

        //endwin();
        ////for(int i = 0; i < 100;i ++)
        //while(1)
        //{
        ////send_insert(SERVER_SOCKET, 0, 0, 'A' + i);
        //send_insert(SERVER_SOCKET, 0, 0, 'A');
        //usleep(1000);
        //}
        //send_write(SERVER_SOCKET, "out.txt");
        //while(1);

        if(ret)
        {
            endwin();
            log("Error in setup! code: %d", ret);
            return ret;
        }

        //waddstr(mainWindow, "hello world!");
        //waddstr(commandWindow, "hello world!");

        wmove(commandWindow, 0, 0);
        //box(commandWindow, 0, 0);
        //box(mainWindow, 0, 0);

        wrefresh(mainWindow);
        wrefresh(commandWindow);

        //delwin(stdscr);

        currWindow = mainWindow;

        wrefresh(currWindow);

        thread_messageHandler = std::thread(handleMessages);
        thread_getFull = std::thread(getFullLoop);

        //while(1);

        if(!START_SERVER)
        {
            send_get_full(SERVER_SOCKET);
        }

        int in;     // a char, but uses higher values for special chars
        while(1)
        {
            //handleMessages();

            int maxx, maxy;
            getmaxyx(mainWindow, maxy, maxx);

            //log("reading char");
            in = wgetch(currWindow);
            //log("read char: %d", in);

            glock.lock();
            mlock.lock();

            if((SERVER_LOST || SHUTDOWN_NETWORK) && !HANDLED_SERVER_LOST)
                handle_disconnect();


            if(in == CTRL_Q)        // exit on CTRL+Q
            {
                log("client got normal quit command");
                mlock.unlock();
                glock.unlock();
                break;
            }
            else if (in == KEY_UP)
            {
                move_win_rel(currWindow, 0, -1);
            }
            else if (in == KEY_DOWN)
            {
                move_win_rel(currWindow, 0, 1);
            }
            else if (in == KEY_LEFT)
            {
                move_win_rel(currWindow, -1, 0);
            }
            else if (in == KEY_RIGHT)
            {
                move_win_rel(currWindow, 1, 0);
            }
            else if (in == ENTER_KEY)
            {
                if(currWindow == mainWindow)
                {
                    int x, y;
                    getyx(mainWindow, y, x);
                    numlines++;
                    if(numdisplaylines < maxy)
                        numdisplaylines++;

                    //text.insert(y + lineoffset, x, in);
                    insert_char(y + lineoffset, x, in);

                    move_win_rel(currWindow, 0, 1);
                    reset_x(currWindow, true);

                }
                else
                {
                    // do special stuff
                    if(commands[0] == ':')
                    {
                        std::stringstream str;
                        for(auto it = commands.begin()+1; it != commands.end(); it++)
                            str << (char)*it;

                        //print_in_cmd_window(str.str().c_str());

                        std::vector<std::string> v = split(str.str(), ' ');
                        v.pop_back();

                        //for(auto& s : v)
                        //print_in_cmd_window(s.c_str());

                        if(v.size() > 0)
                        {
                            if(v[0] == "w")
                            {
                                //print_in_cmd_window(v[1].c_str());

                                // save
                                if(v.size() == 2)
                                {
                                    /*
                                       text.writeToFile(v[1]);
                                       char* c;

                                       asprintf(&c, "Saved file: %s", v[1].c_str());
                                       print_in_cmd_window(c);
                                       free(c);        // I think it uses malloc
                                       */

                                    log("Writing file %s", v[1].c_str());

                                    send_write(SERVER_SOCKET, v[1]);
                                }
                                else
                                {
                                    // bad args
                                    print_in_cmd_window("Bad # of args: ");
                                    print_in_cmd_window(std::to_string(v.size()).c_str());
                                }
                            }
                            else if(v[0] == "e")
                            {
                                if(v.size() == 2)
                                {
                                    /*
                                       size_t newlines = text.readFromFile(v[1]);
                                       char* c;

                                       move_win_rel(mainWindow, -9999999, -99999999);

                                       numlines = newlines;
                                       if(numlines > maxy)
                                       {
                                       numdisplaylines = maxy;
                                       }
                                       else
                                       {
                                       numdisplaylines = numlines;
                                       }

                                       wclear(mainWindow);

                                       asprintf(&c, "Read from file: %s, %lu lines", v[1].c_str(), newlines);
                                       print_in_cmd_window(c);
                                       free(c);        // I think it uses malloc
                                       */

                                    send_read(SERVER_SOCKET, v[1]);
                                }
                                else
                                {
                                    // bad args
                                    print_in_cmd_window("Bad # of args: ");
                                    print_in_cmd_window(std::to_string(v.size()).c_str());
                                }    
                            }
                            else if (v[0] == "wl")
                            {
                                // write local
                                if(v.size() == 2)
                                {
                                    text.writeToFile(v[1]);
                                    char* c;

                                    asprintf(&c, "Saved file: %s", v[1].c_str());
                                    print_in_cmd_window(c);
                                    free(c);        // I think it uses malloc

                                    //log("Writing file %s", v[1].c_str());

                                    //send_write(SERVER_SOCKET, v[1]);
                                }
                                else
                                {
                                    // bad args
                                    print_in_cmd_window("Bad # of args: ");
                                    print_in_cmd_window(std::to_string(v.size()).c_str());
                                }
                            }

                        }


                        //cerr << str.str() << endl;
                    }


                    // clear command window
                    clear_cmd_window();
                }

            }
            else if (in == CTRL_W)
            {
                currWindow = (currWindow == mainWindow ? commandWindow : mainWindow);
            }
            else if (in == KEY_BACKSPACE)
            {
                int x, y;
                getyx(currWindow, y, x);

                if(x > 0)
                {
                    move_win_rel(currWindow, -1, 0);

                    getyx(currWindow, y, x);

                    if(currWindow == commandWindow)
                    {
                        //commands[x] = ' ';
                        commands.erase(commands.begin() + x);
                        commands.push_back(' ');
                    }
                    else
                    {
                        //data[y][x] = ' ';
                        //data[y].erase(data[y].begin() + x);
                        //data[y].push_back(' ');
                        //text.remove(y+lineoffset, x);
                        remove_char(y + lineoffset, x);
                    }
                }
                else // x == 0
                {
                    if(y > 0 && currWindow == mainWindow)
                    {
                        // save width of line above us
                        size_t above_width = text.line_width(y-1);
                        size_t us_width = text.line_width(y);

                        // get the container to remove the line
                        //text.remove(y+lineoffset, -1);
                        remove_char(y + lineoffset, -1);

                        // only subtract numdisplaylines if we are running out of lines
                        if(numlines == numdisplaylines)
                            numdisplaylines--;
                        numlines--;

                        move_win_rel(mainWindow, above_width, -1);
                    }
                }

            }
            else if (in == DELETE_KEY)
            {
                // like backspace but the other way
                int x, y;
                getyx(currWindow, y, x);

                if(currWindow == commandWindow)
                {
                    //commands[x] = ' ';
                    commands.erase(commands.begin() + x);
                    commands.push_back(' ');
                    goto END;
                }

                if(x < text.line_width(y))
                {
                    //data[y][x] = ' ';
                    //data[y].erase(data[y].begin() + x);
                    //data[y].push_back(' ');
                    //text.remove(y+lineoffset, x);
                    remove_char(y + lineoffset, x);
                }
                else // x is last character
                {
                    if(y < numlines-1)
                    {
                        // save width of line above us
                        size_t below_width = text.line_width(y+1);
                        size_t us_width = text.line_width(y);

                        // get the container to remove the line
                        //text.remove(y+lineoffset+1, -1);
                        remove_char(y + lineoffset + 1, -1);

                        // only subtract numdisplaylines if we are running out of lines
                        if(numlines == numdisplaylines)
                            numdisplaylines--;
                        numlines--;
                    }
                }
            }
            else if (isprint(in))
            {
                int x, y;
                getyx(currWindow, y, x);

                if(currWindow == commandWindow)
                {
                    commands[x] = in;
                }
                else
                {
                    //data[y][x] = in;
                    //text.insert(y + lineoffset, x, in);
                    insert_char(y + lineoffset, x, in);
                }

                move_win_rel(currWindow, 1, 0);
            }
            else if (in == 410)
            {
                // happen on resizes, appears to break stuff
            }
            else
            {
                // ugly character

                // this cerr used to get the keycodes of things
                // that aren't already handled, like CTRL+W, etc
                //std::cerr << in << std::endl;

                if(in != -1)
                    log("Client got unhandled char: %d (%c)", in, (char)in);
            }
END:

            refresh_screen();

            glock.unlock();
            mlock.unlock();
        }

        endwin();

        log("Client exiting normally");

        // tell those threads to die
        SERVER_LOST = true;
        SHUTDOWN_NETWORK = true;

        // force that persistent thread to die
        int cret = close(SERVER_SOCKET);
        if(cret)
            perror("close");

        thread_messageHandler.join();
        thread_getFull.join();

        log("Client joined threads normally");

        return 0;
    }
}

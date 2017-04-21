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

#define CTRL_W 23
// the KEY_ENTER is wrong, use this constant instead
#define ENTER_KEY 13
#define CTRL_Q 17
#define DELETE_KEY 330
#define MAC_BACK 127

WINDOW* mainWindow;
WINDOW* commandWindow;
WINDOW* currWindow;

//vector<vector< int >> data;
TextContainer<LockFreeList> text;
std::vector<int> commands;

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

void reset_x(WINDOW* win, bool notify = true)
{
    int x, y;
    getyx(win, y, x);
    wmove(win, y, 0);

    if(win == mainWindow && notify)
        text.move(y + lineoffset, 0);
}

void clear_cmd_window()
{
    for(auto& c : commands)
        c = ' ';
    waddstr(commandWindow, "                                                                                                                                              ");
    // don't notify because this is command window stuff
    reset_x(currWindow, false);
}

void print_in_cmd_window(const char* c)
{
    clear_cmd_window();
    reset_x(commandWindow, false);
    waddstr(commandWindow, c);
    wrefresh(commandWindow);
    sleep(1);
    clear_cmd_window();
    wrefresh(commandWindow);
}

void refresh_screen()
{
    int sx_main, sy_main, sx_command, sy_command;
    int maxx, maxy;
    getmaxyx(stdscr, maxy, maxx);

    getyx(mainWindow, sy_main, sx_main);
    getyx(commandWindow, sy_command, sx_command);

    //for (size_t y = 0; y < numdisplaylines; y++)
    //{
    //    text.print(lineoffset + y, maxx);
    //}

    text.print(lineoffset, maxx);

    for(int x = 0; x < commands.size(); x++)
    {
        // 0 is y coord in the window
        mvwaddch(commandWindow, 0, x, commands[x]);
    }

    wmove(mainWindow, sy_main, sx_main);
    wmove(commandWindow, sy_command, sx_command);

    wrefresh(mainWindow);
    wrefresh(commandWindow);
    wrefresh(currWindow);       // the cursor is drawn on refresh

}

void move_win_rel(WINDOW* win, int xoffs, int yoffs)
{
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
        text.move(capped_y + lineoffset, capped_x);
    }
    else
    {
        // command window
        int capped_x = std::min(x + xoffs, (int)commands.size());

        wmove(win, 0, capped_x);
    }
}

void resize_handler(int sigwinch)
{
    endwin();
    refresh();
    clear();

    int  w, h;
    getmaxyx(stdscr, h, w);

    wresize(mainWindow, h - 1, w);
    wresize(commandWindow, 1, w);

    if(h-1 < numdisplaylines)
    {
        numdisplaylines = h-1;
    }

    /*
    data.resize(h-1);
    for(auto& v : data)
        v.resize(w, ' ');
    */
    commands.resize(w, ' ');

    refresh_screen();
}

void setup()
{
    initscr();      // Init the library
    //cbreak();       // set it up so we read a character at a time
    raw();          // set it up so we read a character at a time
    nonl();
    noecho();       // prevent the screen from showing typed in characters

    use_extended_names(TRUE);

    signal(SIGWINCH, resize_handler);

    wrefresh(stdscr);

    int w, h;
    getmaxyx(stdscr, h, w);

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
}


int main(int argc, char** argv)
{
    setup();

    //waddstr(mainWindow, "hello world!");
    //waddstr(commandWindow, "hello world!");

    FILE* f = fopen("clogerr.txt", "w+");
    dup2(fileno(f), 2);

    wmove(commandWindow, 0, 0);
    //box(commandWindow, 0, 0);
    //box(mainWindow, 0, 0);

    wrefresh(mainWindow);
    wrefresh(commandWindow);

    //delwin(stdscr);

    currWindow = mainWindow;

    char s[2];
    s[1] = '\0';
    int in;     // a char, but uses higher values for special chars
    while(1)
    {
        int maxx, maxy;
        getmaxyx(mainWindow, maxy, maxx);

        in = wgetch(currWindow);

        if(in == CTRL_Q)        // exit on CTRL+Q
            break;
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

                text.insert(y + lineoffset, x, in);

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
                                FILE* file = fopen(v[1].c_str(), "w");
                                for(auto& v1 : data)
                                {
                                    for(auto& c : v1)
                                    {
                                        fwrite(&c, 1, 1, file);
                                    }
                                    fwrite("\n", 1, 1, file);
                                }

                                fclose(file);
                                */
                                text.writeToFile(v[1]);
                                char* c;

                                asprintf(&c, "Saved file: %s", v[1].c_str());
                                print_in_cmd_window(c);
                                free(c);        // I think it uses malloc
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
        else if (in == KEY_BACKSPACE || in==MAC_BACK)
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
                    text.remove(y+lineoffset, x);
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
                    text.remove(y+lineoffset, -1);

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
                text.remove(y+lineoffset, x);
            }
            else // x is last character
            {
                if(y < numlines-1)
                {
                    // save width of line above us
                    size_t below_width = text.line_width(y+1);
                    size_t us_width = text.line_width(y);

                    // get the container to remove the line
                    text.remove(y+lineoffset+1, -1);

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
                text.insert(y + lineoffset, x, in);
            }

            move_win_rel(currWindow, 1, 0);
        }
        else
        {
            // ugly character

            // this cerr used to get the keycodes of things
            // that aren't already handled, like CTRL+W, etc
            //std::cerr << in << std::endl;
        }
END:

        refresh_screen();
    }

    endwin();

    return 0;
}

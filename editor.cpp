#include <curses.h>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <signal.h>
#include <vector>
#include <cstdio>
#include "textcontainer.h"

using namespace std;

#define CTRL_W 23
// the KEY_ENTER is wrong, use this constant instead
#define ENTER_KEY 13
#define CTRL_Q 17

WINDOW* mainWindow;
WINDOW* commandWindow;
WINDOW* currWindow;

vector<vector< int >> data;
vector<int> commands;

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

void reset_x(WINDOW* win)
{
    int x, y;
    getyx(win, y, x);
    wmove(win, y, 0);
}

void clear_cmd_window()
{
    for(auto& c : commands)
        c = ' ';
    waddstr(commandWindow, "                                                          ");
                
    reset_x(currWindow);
}

void print_in_cmd_window(const char* c)
{
    clear_cmd_window();
    reset_x(commandWindow);
    waddstr(commandWindow, c);
    wrefresh(commandWindow);
    sleep(1);
    clear_cmd_window();
    wrefresh(commandWindow);
}

void refresh_screen()
{
    int sx_main, sy_main, sx_command, sy_command;
    getyx(mainWindow, sy_main, sx_main);
    getyx(commandWindow, sy_command, sx_command);

    for (int y = 0; y < data.size(); y++)
    {
        for(int x = 0; x < data[0].size(); x++)
        {
            mvwaddch(mainWindow, y, x, data[y][x]);
        }
    }

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
    wmove(win, y+yoffs, x+xoffs);

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

    data.resize(h-1);
    for(auto& v : data)
        v.resize(w, ' ');
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

    data.resize(h-1);
    for(auto & v : data)
        v.resize(w, ' ');    
    commands.resize(w, ' ');

    nodelay(mainWindow, FALSE);
    nodelay(commandWindow, FALSE);


    wrefresh(mainWindow);
    wrefresh(commandWindow);

    
    keypad(mainWindow, TRUE); // we should handle the special characters ourselves
    keypad(commandWindow, TRUE);
    keypad(stdscr, TRUE);
}


int main(int argc, char** argv)
{
    setup();

    //waddstr(mainWindow, "hello world!");
    //waddstr(commandWindow, "hello world!");

    wmove(commandWindow, 0, 0);
    //box(commandWindow, 0, 0);
    //box(mainWindow, 0, 0);

    wrefresh(mainWindow);
    wrefresh(commandWindow);

    //delwin(stdscr);

    currWindow = commandWindow;

    char s[2];
    s[1] = '\0';
    int in;     // a char, but uses higher values for special chars
    while(1)
    {
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
                move_win_rel(currWindow, 0, 1);
                reset_x(currWindow);
            }
            else
            {
                // do special stuff
                if(commands[0] == ':')
                {
                    stringstream str;
                    for(auto it = commands.begin()+1; it != commands.end(); it++)
                        str << (char)*it;

                    //print_in_cmd_window(str.str().c_str());

                    vector<string> v = split(str.str(), ' ');
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
                                char* c;

                                asprintf(&c, "Saved file: %s", v[1].c_str());
                                print_in_cmd_window(c);
                                free(c);        // I think it uses malloc
                            }
                            else
                            {
                                // bad args
                                print_in_cmd_window("Bad # of args: ");
                                print_in_cmd_window(to_string(v.size()).c_str());
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
                    data[y].erase(data[y].begin() + x);
                    data[y].push_back(' ');
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
                data[y][x] = in;
            }

            move_win_rel(currWindow, 1, 0);
        }
        else
        {
            // ugly character

            // this cerr used to get the keycodes of things that aren't already handled, like CTRL+W, etc
            //cerr << in << endl;
        }


        refresh_screen();
    }

    endwin();
}


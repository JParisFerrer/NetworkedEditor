#include <curses.h>
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <vector>

using namespace std;

#define CTRL_W 23
// the KEY_ENTER is wrong, use this constant instead
#define ENTER_KEY 13

WINDOW* mainWindow;
WINDOW* commandWindow;
WINDOW* currWindow;

vector<vector< int >> data;
vector<int> commands;

void resize_handler(int sigwinch)
{
    int  w, h;
    getmaxyx(stdscr, h, w);

    wresize(mainWindow, h - 1, w);
    wresize(commandWindow, 1, w);
}

void setup()
{
    initscr();      // Init the library
    cbreak();       // set it up so we read a character at a time
    //raw();
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
        v.resize(w, 32);    // 32 is a space
    commands.resize(w,32);     // 32 is a space

    nodelay(mainWindow, FALSE);
    nodelay(commandWindow, FALSE);


    wrefresh(mainWindow);
    wrefresh(commandWindow);

    
    keypad(mainWindow, TRUE); // we should handle the special characters ourselves
    keypad(commandWindow, TRUE);
    keypad(stdscr, TRUE);
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

        if(in == 'q')
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
                int x, y;
                getyx(currWindow, y, x);
                wmove(currWindow, y, 0);    // we already rel_moved the y, so only reset x to 0
            }
            else
            {
                // do special stuff

                // clear command window
                for(auto& c : commands)
                    c = 23;         // space == 23
            }
            
        }
        else if (in == CTRL_W)
        {
            currWindow = (currWindow == mainWindow ? commandWindow : mainWindow);
        }
        else
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


        refresh_screen();
    }

    endwin();
}


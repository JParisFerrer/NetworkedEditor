#include <curses.h>
#include <iostream>
#include <unistd.h>
#include <signal.h>

using namespace std;

#define CTRL_W 23

WINDOW* mainWindow;
WINDOW* commandWindow;

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
    nonl();
    noecho();       // prevent the screen from showing typed in characters

    use_extended_names(TRUE);

    signal(SIGWINCH, resize_handler);

    wrefresh(stdscr);

    int w, h;
    getmaxyx(stdscr, h, w);

    mainWindow = newwin(h - 1, w, 0, 0);
    commandWindow = newwin(1, w, h-1, 0);

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

    waddstr(mainWindow, "hello world!");
    waddstr(commandWindow, "hello world!");

    wmove(commandWindow, 0, 0);
    //box(commandWindow, 0, 0);
    //box(mainWindow, 0, 0);

    wrefresh(mainWindow);
    wrefresh(commandWindow);

    //delwin(stdscr);

    WINDOW* currWindow = commandWindow;

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
            int x, y;
            getyx(currWindow, y, x);
            wmove(currWindow, y-1, x);

        }
        else if (in == KEY_DOWN)
        {
            int x, y;
            getyx(currWindow, y, x);
            wmove(currWindow, y+1, x);

        }
        else if (in == KEY_LEFT)
        {
            int x, y;
            getyx(currWindow, y, x);
            wmove(currWindow, y, x-1);

        }
        else if (in == KEY_RIGHT)
        {
            int x, y;
            getyx(currWindow, y, x);
            wmove(currWindow, y, x+1);
        }
        else if (in == CTRL_W)
        {
            currWindow = (currWindow == mainWindow ? commandWindow : mainWindow);
        }
        else
        {
            s[0] = in;
            //cerr << in << endl;
            waddstr(currWindow, s);
        }

        wrefresh(mainWindow);
        wrefresh(commandWindow);
        wrefresh(currWindow);       // the cursor is drawn on refresh
    }

    endwin();
}


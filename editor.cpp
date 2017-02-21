#include <curses.h>
#include <iostream>

using namespace std;

WINDOW* mainWindow;
WINDOW* commandWindow;


void setup()
{
    initscr();      // Init the library
    cbreak();       // set it up so we read a character at a time
    noecho();       // prevent the screen from showing typed in characters

    wrefresh(stdscr);

    int w, h;
    getmaxyx(stdscr, h, w);

    mainWindow = newwin(h - 1, w, 0, 0);
    commandWindow = newwin(1, w, h-1, 0);


    wrefresh(mainWindow);
    wrefresh(commandWindow);

    
    keypad(mainWindow, TRUE); // we should handle the special characters for the typing window, but not the command window
}

int main(int argc, char** argv)
{
    setup();

    waddstr(mainWindow, "hello world!");
    waddstr(commandWindow, "hello world!");


    wrefresh(mainWindow);
    wrefresh(commandWindow);

    getch();

    endwin();
}


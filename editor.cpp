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

    mainWindow = newwin(LINES - 1, COLUMNS, 0, 0);
    commandWindow = newwin(1, COLUMNS, LINES-1, 0);
    
    keypad(mainWindow, TRUE); // we should handle the special characters for the typing window, but not the command window
}

int main(int argc, char** argv)
{
    setup();

    while(1)
    {
        wrefresh(mainWindow);
        wrefresh(commandWindow);
    }
}


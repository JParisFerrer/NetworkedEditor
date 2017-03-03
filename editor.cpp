#include "container.h"
#include <cstdlib>
#include <curses.h>
#include <unistd.h>

#define ESC 27

#define UP 72
#define DOWN 80
#define LEFT 75
#define RIGHT 77

void print(TextContainer &window) {
  std::system("clear");
  window.print();
}




//^[[Aup^[[Dleft^[[Cright^[[Bdown



int main () {
  fflush(stdin);
  TextContainer window;



  print(window);

  while (true) {

    // mask input echoing while gettign char
    // if its not an arrow key call insert
    // if it is an arrow key check if we can move the cursor
    //
    //   if we cant move the cursor do nothing
    //   if we can print a [ instead of the vector
    // print[
    // for cursor
    char c =getchar();

    if(c==UP||c==DOWN||c==LEFT||c==RIGHT||c==ESC){
      getchar();
      getchar();
      getchar();

      window.move(c);
    }

    else if(c!='\n'&&c!='\r'){
      getchar();
      window.insert(c);

    }
    else{
      window.insert('\n');
    }

    print(window);
    window.toFile();
  }

}

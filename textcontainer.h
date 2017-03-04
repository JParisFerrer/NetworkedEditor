//
#ifndef _TEXTCONTAINER_H_
#define _TEXTCONTAINER_H_

template <typename T>
class TextContainer {
public:
  /*Constructors*/=
    TextContainer();

  /*Core Functions*/

    void add(size_t line, size_t index, int input);
    void remove(size_t line, size_t index);
    void writeToFile(string fileName);
    void readFromFile(string fileName);
    void print(size_t line,size_t maxWidth);

  /*debugging utilties*/

    void printDebug();
    void writeToFileDebug();

private:
  T container;

};

#include "textcontainer.cpp"

#endif

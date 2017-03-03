//
#ifndef _TEXTCONTAINER_H_
#define _TEXTCONTAINER_H_

template <typename T>
class TextContainer {
public:
  void add();
  void remove();
  void moveCursor();
  void save();
  void writeToFile();

  /*debugging utilties*/
  void print();

private:
  T container;

};

#include "textcontainer.cpp"

#endif

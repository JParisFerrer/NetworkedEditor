#ifndef CONTAINER_H
#define CONTAINER_H


#include <vector>
#include <string>
#include <iostream>
#include <fstream>

/*
* @todo  eventually all uses of the data vector will be replaced
*/

class TextContainer {

public:
    TextContainer();
    TextContainer(std::string file);
    ~TextContainer();
    void insert(char input);
    void remove();
    void find();
    void print();
    void move(char direction);

/* Debugging utilities*/

    void toFile();

private:
  std::vector<std::vector< char> > data;
  size_t line; //if newline incr this and push it back always start at index 0
  size_t index;

};



//if the yser tries to go to the end at its a newline take them to index0 line +1

#include "container.cpp"
#endif

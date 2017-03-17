//
#ifndef _LOCKFREELIST_H_
#define _LOCKFREELIST_H_

#include <unordered_map>

struct Location {
    size_t index;
    size_t line;
};

class LockFreeList {
public:
  /*Constructors*/
  LockFreeList();

  /*Core Functions*/
  void insert(size_t line, size_t index, int input);
  void remove(size_t line, size_t index);
  void move(size_t line, size_t index);
  void writeToFile(std::string fileName);
  void readFromFile(std::string fileName);
  void print(size_t line,size_t maxWidth);

  /*debugging utilties*/
  void printDebug();
  void writeToFileDebug();

private:
    int** data;
    int** bufferPool;

    std:unordered_map< size_t, Location > locations;
};

#include "lockfreelist.cpp"

#endif

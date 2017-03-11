//
#ifndef _BLOCKINGVECTOR_H_
#define _BLOCKINGVECTOR_H_



#include <vector>
#include <fstream>
#include <iostream>
#include <mutex>
class BlockingVector {
public:
  /*Constructors*/
  BlockingVector();

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
    std::vector< std::vector<int> > data;

    std::mutex vectorLock;
    std::mutex debugLock;

    size_t index;   // not necessarily needed for blocking vector
    size_t line;
};

#include "blockingVector.cpp"

#endif

//
#ifndef _BLOCKINGVECTOR_H_
#define _BLOCKINGVECTOR_H_


class BlockingVector {
public:
  /*Constructors*/
  BlockingVector();

  /*Core Functions*/
  void insert(size_t line, size_t index, int input);
  void remove(size_t line, size_t index);
  void writeToFile(std::string fileName);
  void readFromFile(std::string fileName);
  void print(size_t line,size_t maxWidth);

  /*debugging utilties*/
  void printDebug();
  void writeToFileDebug();

private:
    std::vector< std::vector<int> > data;

};

#include "blockingVector.cpp"

#endif

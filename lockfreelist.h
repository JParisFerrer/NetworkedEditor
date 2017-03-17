//
#ifndef _LOCKFREELIST_H_
#define _LOCKFREELIST_H_

#include <unordered_map>
#include <thread>
#include <chrono>

#define CHARBUFFER 4
#define UNUSEDINT 99999

struct Location {
    size_t index;
    size_t line;
};

struct BufferList {
    BufferList * next;
    int * buffer;

    BufferList(){
        next=NULL;
        buffer=new int[CHARBUFFER];
    }

    ~BufferList(){
        delete [] buffer;
    }
};

class LockFreeList {
public:
  /*Constructors*/
  LockFreeList();
  ~LockFreeList();

  /*Core Functions*/
  void insert(size_t line, size_t index, int input);
  void remove(size_t line, size_t index);
  void move(size_t line, size_t index);
  void writeToFile(std::string fileName);
  void readFromFile(std::string fileName);
  void print(size_t line,size_t maxWidth);

  /*helpers*/
  void bufferMaker();
  size_t bufferLength();


  /*debugging utilties*/
  void printDebug();
  void writeToFileDebug();

private:
    std::atomic< int ** > data;

    //std::atomic<node<T>*> head;
    std::atomic< BufferList * > bufferPoolHead;

    std:unordered_map< size_t, Location > locations;
};

#include "lockfreelist.cpp"

#endif

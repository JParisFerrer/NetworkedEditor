//
#ifndef _LOCKFREELIST_H_
#define _LOCKFREELIST_H_

#include <unordered_map>
#include <thread>
#include <atomic>
#include <fstream>
#include <iostream>
#include <chrono>
#include "curses.h"
#include <cstring>
#include <regex>
#include <utility>
#include "networking.h"

#define CHARBUFFER 4
#define BUFFERLEN 4
#define UNUSEDINT 999999

struct Location {
    size_t index;
    size_t line;
};

struct Buffer {
    Buffer * next;
    int * buffer;

    Buffer(){
        next=nullptr;
        buffer=new int[CHARBUFFER];
        for(int i = 0; i < CHARBUFFER; i++)
            buffer[i] = UNUSEDINT;
    }

    Buffer(int* buf)
    {
        next = nullptr;
        buffer = buf;
        for(int i = 0; i < CHARBUFFER; i++)
            buffer[i] = UNUSEDINT;
    }

    ~Buffer(){
        // jk we need this later
        delete [] buffer;
    }
};
struct BufferList {
    BufferList *next;
    Buffer * line;
    size_t lineLength;
    size_t lineCapacity;
    BufferList(){
        next=nullptr;
        line=nullptr;
        lineLength=0;
        lineCapacity=0;
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
  size_t readFromFile(std::string fileName);
  void print(WINDOW* win, size_t line,size_t maxWidth, size_t maxHeight);
  void printColored(WINDOW* win, std::string text);

  std::pair<char*, size_t> serialize();
  size_t deserialize(char* buf, size_t len);

  size_t line_width(size_t y);

  /*helpers*/
  void bufferMaker();
  size_t bufferLength();
  void clear();

  /*debugging utilties*/
  void printDebug();
  void writeToFileDebug();
  bool contains(std::vector<std::pair<size_t, size_t>> & v, size_t s);

private:

    BufferList * head;
    size_t dataLength; //CANT USE THIS IF WE WANNA BE TRULLY LOCK FREE


    //std::atomic<node<T>*> head;
    std::atomic< BufferList * > bufferPoolHead;

    std::unordered_map< size_t, Location > locations;

    bool makeLine( BufferList*& currentLine);

    bool getLine(size_t line, BufferList *& currentLine);

    void insertIntoLine(BufferList* line, size_t index, int c);

    int* getBuffer();
};

#endif

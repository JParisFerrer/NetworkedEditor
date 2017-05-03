//
#ifndef _BLOCKINGVECTOR_H_
#define _BLOCKINGVECTOR_H_

#include "networking.h"
#include <cctype>
#include <curses.h>
#include <fstream>
#include <iostream>
#include <mutex>
#include <regex>
#include <utility>
#include <vector>

class BlockingVector {
public:
  /*Constructors*/
  BlockingVector();

  /*Core Functions*/
  void insert(size_t line, size_t index, int input);
  void remove(size_t line, size_t index);
  void move(size_t line, size_t index);
  size_t line_width(size_t line);

  void writeToFile(std::string fileName);
  size_t readFromFile(std::string fileName);
  void print(WINDOW *win, size_t line, size_t maxWidth);
  void printColored(WINDOW *win, std::string text);

  std::pair<char *, size_t> serialize();
  size_t deserialize(char *buf, size_t len);

  /*debugging utilties*/
  void printDebug();
  void writeToFileDebug();

private:
  std::vector<std::vector<int>> data;

  std::mutex vectorLock;
  std::mutex debugLock;

  size_t index; // not necessarily needed for blocking vector
  size_t line;
};

#endif

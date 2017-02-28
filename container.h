#include <vector>
#include <string>

template<typename T>
class TextContainer {

public:
    TextContainer();
    TextContainer(string file);
    void insert();
    void remove();
    void find();

/* Debugging utilities*/
    print();

private:
  std::vector<vector<string>> data;
  size_t line; //if newline incr this and push it back always start at index 0
  size_t index;

};


#include "container.cpp"

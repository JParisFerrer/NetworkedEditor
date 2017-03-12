//
#ifndef _TEXTCONTAINER_H_
#define _TEXTCONTAINER_H_

template <typename T>
class TextContainer {
    public:
        /*Constructors*/
        TextContainer();

        /*Core Functions*/

        void insert(size_t line, size_t index, int input);
        void remove(size_t line, size_t index);
        void move(size_t line, size_t index);
        size_t line_width(size_t line);

        void writeToFile(std::string fileName);
        void readFromFile(std::string fileName);
        void print(size_t line,size_t maxWidth);

        /*debugging utilties*/

        void printDebug();
        void writeToFileDebug();

    private:
        T container;


};

#include "textcontainer.cpp"

#endif

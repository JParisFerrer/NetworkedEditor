#include "container.h"


/*
* Constructor for TextContainer
*
*
*/

TextContainer::TextContainer(){
  line=0;
  index=0;

  this->data.push_back(std::vector<char> ());
}

/*
* Constructor for TextContainer +existing file data
*
* @todo ?avoid parsing the string in the other function?
*   ?insert in a better way?
*/

TextContainer::TextContainer(string file): TextContainer() {
  char * fileData=file.c_str();

  for (size_t i = 0; i < strlen(fileData); i++) {
    data[line].push_back(file[i]);
    if(file[i]=='\n'){
      line++;
      data.push_back(std::vector<char> () );
    }
  }
}


/*
* Constructor for TextContainer +existing file data
*
* @todo change from print to overloaded
*/
void TextContainer::print(){

  for (size_t i = 0; i < this->data.size(); i++) {
    for (size_t j = 0; j < data[i].size(); j++) {
      cout<<data[i][j];

    }
  }

}

/*
* Insert
*
*
*/
void TextContainer::insert(size_t line, size_t index){
//if


}

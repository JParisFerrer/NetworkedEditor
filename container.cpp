


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

TextContainer::~TextContainer(){
  line=0;
  index=0;

}

/*
* Constructor for TextContainer +existing file data
*
* @todo ?avoid parsing the string in the other function?
*   ?insert in a better way?
*/

TextContainer::TextContainer(std::string file): TextContainer() {
  const char * fileData=file.c_str();

  for (size_t i = 0; i < strlen(fileData); i++) {
    this->data[line].push_back(file[i]);
    if(file[i]=='\n'||file[i]=='\r'){
      line++;
      this->data.push_back(std::vector<char> () );
    }
  }
}


/*
* Constructor for TextContainer +existing file data
*
* @todo change from print to overloaded
*/
void TextContainer::print(){

    //std::cout<<"TEST"<<std::endl;


  for (size_t i = 0; i < this->data.size(); i++) {
    for (size_t j = 0; j < this->data[i].size(); j++) {

      if(i==line && j==index){
        std::cout<<'[';
        std::cout<<data[i][j]<<std::flush;;
      }
      else{
        std::cout<<data[i][j]<<std::flush;;
      }
    }
  }

}

/*
* Insert
* @todo ?allow at the clients discretion insertion into a buffer?
*
*/
void TextContainer::insert(char input){
  std::vector<char>::iterator it;
  std::vector< std::vector< char > >::iterator itV;
  if(input=='\n'||input=='\r'){
    // insert the element at line , index if there is anythign remaining in th
    // vector insert it in the new empty vector after taht
    //insert  anew empty vector after that

    it=this->data[line].begin();
    this->data[line].insert(it+index, input);

    //std::cout<<"TEST2"<<std::endl;

    line++;
    index=0;
    itV=this->data.begin();
    this->data.insert(itV+line, std::vector<char> ());
  }
  else {
    it=this->data[line].begin();
    this->data[line].insert(it+index, input);
    index++;
  }



}


void TextContainer::move(char direction) {


  return;
}

/*
* debugging utility will later be converted to save
*/
void TextContainer::toFile(){
  std::ofstream myfile;
  myfile.open ("test.txt");


  for (size_t i = 0; i < this->data.size(); i++) {
    for (size_t j = 0; j < this->data[i].size(); j++) {


          myfile <<(int)data[i][j]<<" "<<std::flush;;

      }
    }



  myfile.close();


}

//TextContainer.cpp

/*Constructors*/
template <typename T>
TextContainer<T>::TextContainer(){

}

/*Core Functions*/
template <typename T>
void TextContainer<T>::insert(size_t line, size_t index, int input){
    this->container.add(line,index,input);
}

template <typename T>
void TextContainer<T>::remove(size_t line, size_t index){
    this->container.remove(line,index);
}

template <typename T>
void TextContainer<T>::writeToFile(std::string fileName){
    this->container.writeToFile(fileName);
}

template <typename T>
void TextContainer<T>::readFromFile(std::string fileName){
    this->container.readFromFile(fileName);
}

template <typename T>
void TextContainer<T>::print(size_t line,size_t maxWidth){
    this->container.print(line, maxWidth);
}

/*debugging utilties*/

template <typename T>
void TextContainer<T>::printDebug(){
    this->container.printDebug();
}

template <typename T>
void TextContainer<T>::writeToFileDebug(){
    this->container.writeToFileDebug();
}

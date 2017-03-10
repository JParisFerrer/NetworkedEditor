#define _NEWLINE 100
#define _CARRIAGE 100
//

/*Constructors*/
BlockingVector::BlockingVector(){


}

/*Core Functions*/
void BlockingVector::insert(size_t line, size_t index, int input){
    std::lock_guard<std::mutex> lock(vectorLock);
    //using auto is like dying inside

    std::vector<int>::iterator itHorz=this->data[line].begin();
    std::vector< std::vector<int> >::iterator itVert=this->data.begin();

    if (input == _NEWLINE || input == _CARRIAGE) {
        this->data[line].insert(itHorz+index, input);
        this->data.insert(itVert+line, std::vector<int> ());
    }
    else{
        this->data[line].insert(itHorz+index, input);
    }

}

void BlockingVector::remove(size_t line, size_t index){
    std::lock_guard<std::mutex> lock(vectorLock);
    /*if the index is 0
        if line is not 0
            delete the new line in line-1
            move the vector to line -1 and delete that line
            move(index to length of lin-1 and line)
    else
        delete that index only */
}
void BlockingVector::move(size_t line, size_t index){
    this->index=index;

}

void BlockingVector::writeToFile(std::string fileName){
    //init file with vector data
    std::ofstream outFile;
    outFile.open(fileName,std::fstream::app);

    for(size_t i = 0; i<this->data.size(); i++){
        for(size_t j = 0; j<this->data[i].size(); j++){
            outFile<<data[i][j];
        }
    }

}

void BlockingVector::readFromFile(std::string fileName){
    std::lock_guard<std::mutex> lock(vectorLock);
    //init vector with file data




    //get int

}

void BlockingVector::print(size_t line,size_t maxWidth){
    std::lock_guard<std::mutex> lock(vectorLock);

    for(size_t i = 0; i<this->data.size(); i++){
        for(size_t j = 0; j<this->data[i].size(); j++){
            std::cout<<data[i][j]<<" ";
        }
    }
}


/*debugging utilties*/
void BlockingVector::printDebug(){
    std::cout<<std::endl;
    for(size_t i = 0; i<this->data.size(); i++){
        for(size_t j = 0; j<this->data[i].size(); j++){
            std::cout<<data[i][j]<<" ";
        }
    }
    std::cout<<std::endl;
}

/*TODO do not like this way of locking!!!!*/
void BlockingVector::writeToFileDebug(){
    std::lock_guard<std::mutex> lock(debugLock);
    static size_t count=0;
    std::ofstream outFile;
    outFile.open("debug.txt",std::fstream::app);

    outFile<<count<<std::endl;
    for(size_t i = 0; i<this->data.size(); i++){
        for(size_t j = 0; j<this->data[i].size(); j++){
            outFile<<data[i][j]<<" ";
        }
    }

    count++;
    outFile.close();
}

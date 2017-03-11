#define _NEWLINE KEY_ENTER
//

/*Constructors*/
BlockingVector::BlockingVector(){
    data.push_back(std::vector<int> ());

}

/*Core Functions*/
void BlockingVector::insert(size_t line, size_t index, int input){
    std::lock_guard<std::mutex> lock(vectorLock);
    //using auto is like dying inside

    std::vector<int>::iterator itHorz=this->data[line].begin();
    std::vector< std::vector<int> >::iterator itVert=this->data.begin();

    if (input == _NEWLINE) {
        this->data[line].insert(itHorz+index, input);
        this->data.insert(itVert+line, std::vector<int> ());
    }
    else{
        this->data[line].insert(itHorz+index, input);
    }

}

void BlockingVector::remove(size_t line, size_t index){
    std::lock_guard<std::mutex> lock(vectorLock);

    std::vector<int>::iterator itHorz=this->data[line].begin();
    std::vector<int>::iterator itHorzEnd=this->data[line].end();
    std::vector< std::vector<int> >::iterator itVert=this->data.begin();

    if( index==0){
        if(line!=0) {
                auto itAbove=this->data[line-1].end();
                data[line-1].insert(itAbove, itHorz,itHorzEnd);
                data.erase(itVert+line);
        }
    }
    else{
        data[line].erase(itHorz+index);
    }
}
void BlockingVector::move(size_t line, size_t index){
    std::lock_guard<std::mutex> lock(vectorLock);
    this->index=index;
    this->line=line;
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
            std::cout<<data[i][j];
        }
        std::cout<<std::endl;
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

#include "blockingVector.h"

extern WINDOW* mainWindow;

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

    if (input == KEY_ENTER) {
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

size_t BlockingVector::line_width(size_t line)
{
    std::lock_guard<std::mutex> lock(vectorLock);
    return this->data[line].size();
}

void BlockingVector::writeToFile(std::string fileName){
    //init file with vector data
    std::ofstream outFile;
    outFile.open(fileName, std::fstream::out);

    for(size_t i = 0; i<this->data.size(); i++){
        for(size_t j = 0; j<this->data[i].size(); j++){
            outFile<<data[i][j];
        }
        outFile << std::endl;
    }

}

void BlockingVector::readFromFile(std::string fileName){
    std::lock_guard<std::mutex> lock(vectorLock);
    std::vector<std::vector<int>> newvec;
    //init vector with file data
    std::fstream in;
    in.open(fileName, std::fstream::in);

    int inchar;
    size_t vecindex = 0;
    newvec.push_back(std::vector<int>());

    while(in.good()){
        in >> inchar;
        if(inchar == '\n'){
            vecindex++;
            newvec.push_back(std::vector<int>());
        }
        else{
            newvec[vecindex].push_back(inchar);
        }
    }
    data = std::move(newvec);

}

void BlockingVector::print(size_t line,size_t maxWidth){
    std::lock_guard<std::mutex> lock(vectorLock);

    for(size_t i = line; i < this->data.size(); i++){
        for(size_t j = 0; j < std::min(this->data[i].size(), maxWidth); j++){
            //std::cout << (char)data[i][j];
            mvwaddch(mainWindow, i+line, j, this->data[i][j]);
        }
        //std::cout<<std::endl;
    }
}


/*debugging utilties*/
/* TODO ?delete?*/
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

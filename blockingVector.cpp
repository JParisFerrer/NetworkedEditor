#include "blockingVector.h"

// the KEY_ENTER is wrong, use this constant instead
#define ENTER_KEY 13

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

    if(line >= data.size())
    {
        fprintf(stderr, "[!!] bad line access! %lu >= %lu\n", line, data.size());
        return;
    }

    if(index > data[line].size())
    {
        fprintf(stderr, "[!!] bad index access! %lu > %lu\n", index, data[line].size());
        return;
    }

    if (input == ENTER_KEY) {
        //this->data[line].insert(itHorz+index, input);
        this->data.insert(itVert+line+1, std::vector<int> ());
        this->data[line+1].assign(itHorz+index, data[line].end());
        //this->data[line].erase(itHorz+index, data[line].end());
        // get new iterators
        this->data[line].erase(this->data[line].begin() + index, data[line].end());
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

    if( index== (size_t)-1){
        if(line!=0) {
            if(line >= data.size())
            {
                fprintf(stderr, "[!!] bad line access! %lu >= %lu\n", line, data.size());
                return;
            }

            if( data[line].size() > 0)
            {
                if(index > data[line].size())
                {
                    fprintf(stderr, "[!!] bad index access! %lu > %lu\n", index, data[line].size());
                    return;
                }
                auto itAbove=this->data[line-1].end();
                data[line-1].insert(itAbove, itHorz,itHorzEnd);
                data.erase(itVert+line);
            }
            else
            {
                data.erase(itVert+line);
            }
        }

    }
    else{
        if(line >= data.size())
        {
            fprintf(stderr, "[!!] bad line access! %lu >= %lu\n", line, data.size());
            return;
        }

        if(index > data[line].size())
        {
            fprintf(stderr, "[!!] bad index access! %lu > %lu\n", index, data[line].size());
            return;
        }

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
    std::lock_guard<std::mutex> lock(vectorLock);
    //init file with vector data
    std::ofstream outFile;
    outFile.open(fileName, std::fstream::out);

    for(size_t i = 0; i<this->data.size(); i++){
        for(size_t j = 0; j<this->data[i].size(); j++){
            outFile << (char)data[i][j];
        }
        outFile << std::endl;
    }

}

size_t BlockingVector::readFromFile(std::string fileName){
    std::lock_guard<std::mutex> lock(vectorLock);
    std::vector<std::vector<int>> newvec;
    //init vector with file data
    std::fstream in;
    in.open(fileName, std::fstream::in);

    std::string line;
    size_t vecindex = 0;
    newvec.push_back(std::vector<int>());

    while(std::getline(in, line)){
        //in >> inchar;
        for(const char & c : line)
        {
            newvec[vecindex].push_back(c);
        }
        vecindex++;
        newvec.push_back(std::vector<int>());
    }
    // the last line we don't need, it didn't actually exist
    newvec.pop_back();
    data = std::move(newvec);

    return data.size();
}

void BlockingVector::print(WINDOW* win, size_t line,size_t maxWidth){
    std::lock_guard<std::mutex> lock(vectorLock);

    size_t index = 0;
    for(size_t i = line; i < this->data.size(); i++, index++){
        // clear screen
        wmove(win, index, 0);
        waddstr(win, "                                                                                                                                                 ");   // clear line
        for(size_t j = 0; j < std::min(this->data[i].size(), maxWidth); j++){
            //std::cout << (char)data[i][j];
            mvwaddch(win, index, j, this->data[i][j]);
        }
        //std::cout<<std::endl;
    }
}

std::pair<char*, size_t> BlockingVector::serialize()
{
    std::lock_guard<std::mutex> lock(vectorLock);

    // serialize ourselves to a new buffer
    // could do realloc but too lazy, so do a two pass

    // add 12 bytes (3 ints) at the beginning, for reasons
    size_t len = 3;

    for(size_t y = 0; y < data.size(); y++)
    {
        for(size_t x = 0; x < data[y].size(); x++)
        {
            len ++;
        }
        len ++;     // add in the newline terminator
    }

    int* ret = new int[len];

    // start at 4th int
    size_t i = 3;

    for(size_t y = 0; y < data.size(); y++)
    {
        for(size_t x = 0; x < data[y].size(); x++)
        {
            ret[i++] = htonl(data[y][x]);
        }

        ret[i++] = htonl((int)'\n');     // add in the newline terminator
    }

    return std::make_pair((char*)ret, (len)*4);
}


size_t BlockingVector::deserialize(char* ibuf, size_t len)
{
    // get a reasonable int array back
    // also skip first 12 bytes (3 ints) for reasons
    int* buf = (int*)(ibuf + 12);
    len /= 4;
    len -= 3;


    // copy into ourselves
    std::lock_guard<std::mutex> lock(vectorLock);

    std::vector<std::vector<int>> newvec;

    size_t vecindex = 0;
    size_t read = 0;
    // throw away first 10 bytes, for reasons
    int* t = buf;

    newvec.push_back(std::vector<int>());

    while(read < len)
    {
        while(ntohl(*t) != '\n')
        {
            newvec[vecindex].push_back(ntohl(*t));
            read++;
            t++;
        }
        vecindex++;
        read++;
        t++;
        newvec.push_back(std::vector<int>());
    }
    // the last line we don't need, it didn't actually exist

    newvec.pop_back();
    data = std::move(newvec);

    return data.size();
}


/*debugging utilties*/
/* TODO ?delete?*/
void BlockingVector::printDebug(){
    std::lock_guard<std::mutex> lock(vectorLock);
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

//
#include "lockfreelist.h"
#include <iostream>

#define CONDITION 1
#define BUFFERLEN 4
#define CHARBUFFER 4
// the KEY_ENTER is wrong, use this constant instead
#define ENTER_KEY 13

extern WINDOW* mainWindow;

/*Constructors*/
LockFreeList::LockFreeList() {
    data=new BufferList();

    data->line= new Buffer();
    data->lineCapacity+=BUFFERLEN;
    data->lineLength=0;
    dataLength = 1;
    //data->line->buffer[0];


    //dataCapacity = 64;

    //std::thread bufferFiller (&LockFreeList::bufferMaker, this);
    //bufferFiller.detach();

    //next = nullptr;
}

LockFreeList::~LockFreeList() {


    //send condition to kill bufferMaker
}

/*Core Functions*/
void LockFreeList::bufferMaker() {
    size_t currLength=0;

    while (CONDITION){
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        size_t buff=bufferLength();
        if(buff<BUFFERLEN){
            for (size_t i = 0; i < BUFFERLEN-buff; i++) {
                BufferList * newBuff=new BufferList();
                //insert bufferPoolHead into newBuff.next
                //exchange newBuff into bufferPoolHead

                // make these atomic
                newBuff->next = bufferPoolHead.load();
                bufferPoolHead = newBuff;
            }
        }
    }
}

int* LockFreeList::getBuffer()
{
    // int* r = new int[CHARBUFFER];
    // for (int i = 0; i < CHARBUFFER; i++)
    //     r[i] = UNUSEDINT;
    //
    // return r;
    //
    // while(bufferPoolHead.load() == nullptr);
    //
    // // make atomic
    // BufferList* ret = bufferPoolHead.load();
    // // make atomic
    // bufferPoolHead = bufferPoolHead.load()->next;
    //
    // //fprintf(stderr, "Returning buffer %p\n", ret->buffer);
    //
    // return ret->buffer;
}

/*swap this out later*/
size_t LockFreeList::bufferLength(){
    // size_t len=0;
    // while(bufferPoolHead.load()!=NULL){
    //     len++;
    //     bufferPoolHead=bufferPoolHead.load()->next;
    // }
    // return len;
}


void LockFreeList::insert(size_t line, size_t index, int input){
/*std::lock_guard<std::mutex> lock(vectorLock);

    next ← p.next
    n.next ← next
    cas(address-of(p.next), next, n)
    If the cas was not successful, go back to 1.


//using auto is like dying inside

std::vector<int>::iterator itHorz=this->data[line].begin();
std::vector< std::vector<int> >::iterator itVert=this->data.begin();

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
*/
//
// data=new BufferList();
//
// data->line= new Buffer();
// data->lineCapacity+=BUFFERLEN;
// data->lineLength=0;
// //dataLength = 1;



    //LockFreeList* lineList = getList(line);

    // if(lineList == nullptr)
    // {
    //     fprintf(stderr, "[!!] Got bad insert line index %lu in %s!\n", line, __func__);
    // }
    if(input == ENTER_KEY)
    {
        BufferList* insertPoint=data;
        int i=0;
        while(insertPoint!=NULL){
            if(i==line) {
                std::cout<<"LINE"<<i<<" ";
                break;
            }
            insertPoint->next=insertPoint;
            i++;
        }
        if(i!=line) {
            fprintf(stderr, "[!!] Bad line of %lu (max len: %lu) in %s\n", line, dataLength * CHARBUFFER, __func__);
            return ;
        }

        //INSERT THE newline
        if(insertPoint->lineCapacity>index){
            std::cout<<"INSERTING IF";
            //traverse to index
            int i=0;
            int buffI=index/CHARBUFFER;
            int buffOff=index%CHARBUFFER;
            Buffer* line=insertPoint->line;
            while(1){
                if(i==buffI) {
                    int j=0;
                    while (1){
                            if(j==buffOff){
                                line->buffer[j]=input;
                                break;
                            }
                            j++;
                    }
                    break;
                }
                i++;
                line=line->next;
            }
        }
        else{
            std::cout<<"INSERTING eLSE";
            // traverse to last buffer and make a new one
            int i=0;
            int buffI=index/CHARBUFFER;
            int buffOff=index%CHARBUFFER;
            Buffer* line=insertPoint->line;
            while(1){
                if(i==buffI) {

                    int j=0;
                    while (1){
                            if(j==buffOff){
                                line->buffer[j]=input;
                                break;
                            }
                            j++;
                    }
                    break;
                }
                i++;
                if(line->next==NULL){
                    line->next=new Buffer();
                }
                line=line->next;

            }
        }
        std::cout<<"INSERTING NL";

        BufferList* newline=new BufferList();

        newline->line= new Buffer();
        newline->lineCapacity+=BUFFERLEN;
        newline->lineLength=0;

        insertPoint->next=newline;
        std::cout<<"INSERTING NL"<<insertPoint<<std::endl;
        return;
        //dataLength++;
        // we need to add that line
        // LockFreeList* last = getList(line);
        //
        // if(last == nullptr)
        // {
        //     fprintf(stderr, "[!!] Missing line with index %lu in %s!\n", line, __func__);
        //     return;
        // }
        // // else we good to make a new one
        // LockFreeList* newlist = new LockFreeList();
        // // make atomic
        // last->next = newlist;

        //newlist->insertInto(index, input);
    }
    else
    {

        BufferList* insertPoint=data;
        int i=0;
        while(insertPoint!=NULL){
            // std::cout<<"This shouldn't be printed"<<std::endl;
            if(i==line) {break;}
            insertPoint->next=insertPoint;
            i++;
        }
        if(i!=line) {
            std::cout<<"This shouldn't be printed"<<std::endl;
        }

        // insertPoint->line->buffer[index%CHARBUFFER]=input;

        if(insertPoint->lineCapacity > index){
            //traverse to index
            std::cerr << "if loop";
            int i=0;
            int buffI=index/CHARBUFFER;
            int buffOff=index%CHARBUFFER;
            Buffer* line=insertPoint->line;
            while(1){
                std::cerr << "i loop";
                if(i==buffI) {
                    int j=0;
                    while (1){
                                std::cerr << "j loop";
                            if(j==buffOff){
                                line->buffer[j]=input;
                                break;
                            }
                            j++;
                    }
                    break;
                }
                i++;
                line=line->next;
            }
        }
        else{
            // traverse to last buffer and make a new one
            std::cerr << "else"; 
            int i=0;
            int buffI=index/CHARBUFFER;
            int buffOff=index%CHARBUFFER;
            Buffer* line=insertPoint->line;
            while(1){

                std::cerr << "i loop";
                if(i==buffI) {

                    int j=0;
                    while (1){
                            if(j==buffOff){
                                line->buffer[j]=input;
                                std::cerr << "j loop";
                                break;
                            }
                            j++;
                    }
                    break;
                }
                i++;
                if(line->next==NULL){
                    line->next=new Buffer();
                }
                line=line->next;

            }
        }
    }
        // we need to insert into the given list
    //     lineList->insertInto(index, input);
    // }


}

void LockFreeList::insertInto(size_t index, int c)
{
    // // actually perform the insertion into buffers and stuff
    // size_t bufindex = index / CHARBUFFER;
    // size_t bufoffset = index % CHARBUFFER;
    //
    // if(bufindex > dataLength)
    // {
    //     fprintf(stderr, "[!!] Bad index of %lu (max len: %lu) in %s\n", index, dataLength * CHARBUFFER, __func__);
    //     return;
    // }
    // // theoretically could be asking for something up to 4 chars after the end of the line, but we trust editor (a bit)
    // if (bufindex == dataLength)
    // {
    //     // need new buffer
    //     if(dataLength < dataCapacity)
    //     {
    //         data[dataLength++] = getBuffer();
    //     }
    //     else
    //     {
    //         // make it bigger
    //         int** newdata = new int*[dataCapacity * 2]();
    //         for(size_t i = 0; i < dataCapacity; i++)
    //             newdata[i] = data[i]; // shallow copy
    //         delete [] data.load();     // proper code would do this after in case of errors
    //         data = newdata;
    //         dataCapacity = 2* dataCapacity;
    //
    //         // then add a buffer
    //         data[dataLength++] = getBuffer();
    //     }
    // }
    // // now the buffer will exist and we can just write in there
    // data[bufindex][bufoffset] = c;


}

void LockFreeList::remove(size_t line, size_t index){

}
void LockFreeList::move(size_t line, size_t index){
    // update the line and index object variables
    // insert a buffer there

}

void LockFreeList::writeToFile(std::string fileName){

}

size_t LockFreeList::readFromFile(std::string fileName){

    return 0;
}

void LockFreeList::print(size_t line,size_t maxWidth){

    // LockFreeList* t = this;
    //
    // size_t index = line;
    // while(t != nullptr)
    // {
    //     wmove(mainWindow, index, 0);
    //     waddstr(mainWindow, "                                                                                                                                                 ");
    //
    //     size_t printindex = 0;
    //     for(ssize_t i = 0; i < maxWidth; i++)
    //     {
    //         size_t bufindex = i / CHARBUFFER;
    //         size_t bufoffset = i % CHARBUFFER;
    //
    //         // if too far, stop this line
    //         if(bufindex >= t->dataLength)
    //         {
    //             break;
    //         }
    //
    //         if(data[bufindex][bufoffset] != UNUSEDINT)
    //         {
    //             mvwaddch(mainWindow, index, printindex, t->data.load()[bufindex] [bufoffset]);
    //             printindex++;
    //         }
    //     }
    //
    //     t = t->next;
    //     index++;
    // }
    BufferList * list=data;
    size_t index = line;
    while(list!=nullptr){
        wmove(mainWindow, index, 0);

        Buffer* lineData=list->line;
        size_t printindex = 0;
        while(lineData!=nullptr){
            for (size_t i = 0; i < CHARBUFFER; i++) {
                if (lineData->buffer[i]!=UNUSEDINT) {
                     mvwaddch(mainWindow, index, printindex, lineData->buffer[i]);

                         printindex++;
                }

            }
            lineData=lineData->next;
        }

        list=list->next;
        index++;
    }


}


/*debugging utilties*/
void LockFreeList::printDebug(){

}

void LockFreeList::writeToFileDebug(){

}

LockFreeList* LockFreeList::getList(size_t line)
{
    // LockFreeList* t = this;
    //
    // while(t)
    // {
    //     if(line == 0)
    //         break;
    //
    //     t = t->next;
    //     line--;
    // }
    //
    // return t;
}

size_t LockFreeList::line_width(size_t y)
{
    // size_t bufindex = y / CHARBUFFER;
    // size_t bufoffset = y % CHARBUFFER;
    //
    // LockFreeList* list = getList(y);
    //
    // return list->dataLength * CHARBUFFER;
}

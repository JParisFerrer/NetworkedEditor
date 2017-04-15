
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
    head=new BufferList();

    head->line= new Buffer();
    head->lineCapacity+=BUFFERLEN;
    head->lineLength=0;
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
        BufferList* insertPoint;
        bool ret = getLine(line, insertPoint);

        if(!ret ) {
            fprintf(stderr, "[!!] Bad line of %lu (max len: %lu) in %s\n", line, dataLength * CHARBUFFER, __func__);
            return ;
        }

        makeLine(insertPoint);

        insertIntoLine(insertPoint, index, input);
    }
    else{
        BufferList* insertPoint;
        bool ret = getLine(line, insertPoint);

        if(!ret ) {
            fprintf(stderr, "[!!] Bad line of %lu (max len: %lu) in %s\n", line, dataLength * CHARBUFFER, __func__);
            return ;
        }

        insertIntoLine(insertPoint, index, input);
    }
    // std::cout<<"INSERTING NL";
    //
    // BufferList* newline=new BufferList();
    //
    // newline->line= new Buffer();
    // newline->lineCapacity+=BUFFERLEN;
    // newline->lineLength=0;
    //
    // insertPoint->next=newline;
    // std::cout<<"INSERTING NL"<<insertPoint<<std::endl;
    // return;
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
    // }
    // else
    // {
    //
    //     BufferList* insertPoint=data;
    //     int i=0;
    //     while(insertPoint!=NULL){
    //         // std::cout<<"This shouldn't be printed"<<std::endl;
    //         if(i==line) {break;}
    //         insertPoint->next=insertPoint;
    //         i++;
    //     }
    //     if(i!=line) {
    //         std::cout<<"This shouldn't be printed"<<std::endl;
    //     }
    //
    //     // insertPoint->line->buffer[index%CHARBUFFER]=input;
    //
    //     if(insertPoint->lineCapacity > index){
    //         //traverse to index
    //         std::cerr << "if loop";
    //         int i=0;
    //         int buffI=index/CHARBUFFER;
    //         int buffOff=index%CHARBUFFER;
    //         Buffer* line=insertPoint->line;
    //         while(1){
    //             std::cerr << "i loop";
    //             if(i==buffI) {
    //                 int j=0;
    //                 while (1){
    //                             std::cerr << "j loop";
    //                         if(j==buffOff){
    //                             line->buffer[j]=input;
    //                             break;
    //                         }
    //                         j++;
    //                 }
    //                 break;
    //             }
    //             i++;
    //             line=line->next;
    //         }
    //     }
    //     else{
    //         // traverse to last buffer and make a new one
    //         std::cerr << "else";
    //         int i=0;
    //         int buffI=index/CHARBUFFER;
    //         int buffOff=index%CHARBUFFER;
    //         Buffer* line=insertPoint->line;
    //         while(1){
    //
    //             std::cerr << "i loop";
    //             if(i==buffI) {
    //
    //                 int j=0;
    //                 while (1){
    //                         if(j==buffOff){
    //                             line->buffer[j]=input;
    //                             std::cerr << "j loop";
    //                             break;
    //                         }
    //                         j++;
    //                 }
    //                 break;
    //             }
    //             i++;
    //             if(line->next==NULL){
    //                 line->next=new Buffer();
    //             }
    //             line=line->next;
    //
    //         }
    //     }
    // }
    //     // we need to insert into the given list
    // //     lineList->insertInto(index, input);
    // // }
    //
    // else{
    //     BufferList*  currentLine=nullptr;
    //     bool lineExisted = getLine(line,currentLine);
    //     if(lineExisted==false){
    //         fprintf(stderr, "[!!] Missing line with index %lu in %s!\n", line, __func_ );
    //         return;
    //     }
    //     insertIntoLine(currentLine,index,c);
    // }

}

void LockFreeList::insertIntoLine(BufferList* line,size_t index, int c)
{
    int buffI=index/CHARBUFFER;
    int buffOff=index%CHARBUFFER;

    Buffer* p = line->line;
    Buffer* toinsert = line->line;

    for(int i = 0; i < buffI; i++)
    {
            toinsert = toinsert->next;

            if(toinsert == nullptr && i < buffI-1)
            {
                fprintf(stderr, "[%s] bad index %lu\n", __func__, index);
                return;
            }
            else if(toinsert == nullptr)
            {
                // allocate a new buffer
                p->next = new Buffer();
                toinsert = p->next;
                line->lineCapacity += BUFFERLEN;
            }

            p = toinsert;
    }
    // toinsert is the buffer to insert into
    toinsert->buffer[buffOff] = c;
    line->lineLength++;
}
//returns false if it doesnt exist and true if it does
bool LockFreeList::makeLine( BufferList*& currentLine) {

    BufferList* newline=new BufferList;
    newline->line=new Buffer();
    newline->lineCapacity += BUFFERLEN;
    newline->lineLength = 0;
    //
    // BufferList* newline=new BufferList();
    //
    // newline->line= new Buffer();
    // newline->lineCapacity+=BUFFERLEN;
    // newline->lineLength=0;

    currentLine->next=newline;
}

//returns false if it doesnt exist and sets currentline to before null
    // and true if it does if current line is null that means head does not exist

bool LockFreeList::getLine(size_t line, BufferList*& currentLine)
{
    BufferList* traverser=head;
    if (traverser==nullptr) {
        fprintf(stderr, "[%s] Head did not exist", __func__ );
        return false;
    }
    int i=0;
    while(traverser->next!=nullptr && (i!=line) ){
        i++;
        traverser=traverser->next;
    }
    currentLine=traverser;
    return (i==line);
}

size_t LockFreeList::line_width(size_t line) {




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
    BufferList * list=head;
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

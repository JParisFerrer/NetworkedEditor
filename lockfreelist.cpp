//
#include "lockfreelist.h"

#define BUFFERLEN 100
#define CONDITION 1


/*Constructors*/
LockFreeList::LockFreeList() {
    data=new int *[64]();

    data[0] = new int[CHARBUFFER];
    for (int i = 0; i < CHARBUFFER; i++)
        data[0][i] = UNUSEDINT;

    dataLength = 0;
    dataCapacity = 64;

    std::thread bufferFiller (bufferMaker);

    next = nullptr;
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

void LockFreeList::getBuffer()
{
    while(bufferPoolHead.load() == nullptr);

    // make atomic
    BufferList* ret = bufferPoolHead.load();
    // make atomic
    bufferPoolHead = bufferPoolHead.load()->next;

    return ret->data;
}

/*swap this out later*/
size_t bufferLength(){
    size_t len=0;
    while(bufferPoolHead!=NULL){
        len++;
        bufferPoolHead=bufferPoolHead->next;
    }
    return len;
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

    LockFreeList* lineList = getList(line);

    if(lineList == nullptr)
    {
        // we need to add that line
        LockFreeList* last = getList(line-1);

        if(last == nullptr)
        {
            fprintf(stderr, "[!!] Got bad index %lu in %s!\n", line, __func__);
            return;
        }
        // else we good to make a new one
        LockFreeList* newlist = new LockFreeList();
        // make atomic
        last->next = newlist;

        newlist->insertInto(index, input);
    }
    else
    {
        // we need to insert into the given list
        lineList->insertInto(index, input);
    }


}

void LockFreeList::insertInto(size_t index, int c)
{
    // actually perform the insertion into buffers and stuff
    size_t bufindex = index / CHARBUFFER;
    size_t bufoffset = index % CHARBUFFER;

    if(bufindex > dataLength)
    {
        fprintf(stderr, "[!!] Bad index of %lu (max len: %lu) in %s\n", index, dataLength * CHARBUFFER, __func__);
        return;
    }
    // theoretically could be asking for something up to 4 chars after the end of the line, but we trust editor (a bit)
    if (bufindex == dataLength)
    {
        // need new buffer
        if(dataLength < dataCapacity)
        {
            data[dataLength++] = getBuffer();
        }
        else
        {
            // make it bigger
            int** newdata = new int*[dataCapacity * 2]();
            for(size_t i = 0; i < dataCapacity; i++)
                newdata[i] = data[i]; // shallow copy
            delete [] data;     // proper code would do this after in case of errors
            data = newdata;

            // then add a buffer
            data[dataLength++] = getBuffer();
        }
    }
    // now the buffer will exist and we can just write in there
    data[bufindex][bufoffset] = c;


}

void LockFreeList::remove(size_t line, size_t index){

}
void LockFreeList::move(size_t line, size_t index){
    // update the line and index object variables
    // insert a buffer there

}

void LockFreeList::writeToFile(std::string fileName){

}

void LockFreeList::readFromFile(std::string fileName){

}

void LockFreeList::print(size_t line,size_t maxWidth){

}


/*debugging utilties*/
void LockFreeList::printDebug(){

}

void LockFreeList::writeToFileDebug(){

}

LockFreeList* LockFreeList::getList(size_t line)
{
    LockFreeList* t = this;

    while(*t)
    {
        if(line == 0)
            break;

        t = t->next;
        line--;
    }

    return t;
}


#include "lockfreelist.h"
#include <iostream>

#define CONDITION 1
#define BUFFERLEN 4
#define CHARBUFFER 4
// the KEY_ENTER is wrong, use this constant instead
#define ENTER_KEY 13

extern WINDOW* mainWindow;

// trim from start
static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}




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
            const char* c = __func__;
            log( "[!!] Bad line of %lu (max len: %lu) in %s", line, dataLength * CHARBUFFER, c);
            return ;
        }

        makeLine(insertPoint);
        

        // copy over rest of line
        size_t in = 0;
        Buffer* t = insertPoint->line;
        int oi = 0;
        int i;
        bool found = false;
        // get to the correct index
        while(1)
        {
            for (i = 0; i < BUFFERLEN; i++)
            {
                if(oi == index)
                {
                    found = true;
                    break;
                }
                if(t->buffer[i] != UNUSEDINT)
                    oi ++;
            }

            if(found)
                break;

            t = t->next;

            if(!t)
            {
                // nothing to copy
                return;
            }
        }

        while(t)
        {
            for ( i = i; i < BUFFERLEN; i++)
            {
                if(t->buffer[i] != UNUSEDINT)
                {
                    insertIntoLine(insertPoint->next, in++, t->buffer[i]);
                    t->buffer[i] = UNUSEDINT;
                }

            }
            i = 0;

            t = t->next;
        }

        //insertIntoLine(insertPoint, index, input);
    }
    else{
        BufferList* insertPoint;
        bool ret = getLine(line, insertPoint);

        if(!ret ) {
            const char* c = __func__;
            log("[!!] Bad line of %lu (max len: %lu) in %s", line, dataLength * CHARBUFFER, c);
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

//    Buffer* p = line->line;
    //std::cout<<index<<std::endl;
    Buffer * currLine = line->line;
    int currIndex=0;
    Buffer * lastBuffer=nullptr;
    size_t i=0;
    bool inserted = false;
    while(currLine!=NULL){
        if (currLine->next==nullptr) {
            lastBuffer=currLine;
        }
        for ( i = 0; i < BUFFERLEN; i++) {
            //fprintf(stderr, "i:%lu curr:%d\n", i, currIndex);
            if (currIndex==index) {
                //std::cout<< "SHOULD WORK";
                //currLine->buffer[i]=c;
                buffOff = i;
                inserted = true;
                break;
            }
            if(currLine->buffer[i]!=UNUSEDINT) {

                currIndex++;
            }

        }
        if (inserted) {
        //    currLine->buffer[i]=c;
            // we exited the for, not break
            break;
        }
        buffI ++;
        currLine=currLine->next;
    }
        // std::cout<< currIndex<<" "<<index<<std::endl;
    //fprintf(stderr, "curr: %d, want: %lu, inserted:%d\n", currIndex, index, inserted);

    // if we hit the end of the line, add it there
    if(!inserted){
        //fprintf(stderr, "NEW BUFF\n");
        //fprintf(stdout, "NEW BUFF\n");

        Buffer* newBuffer =new Buffer();
        newBuffer->buffer[0]=c;
        lastBuffer->next=newBuffer;

    }
    else
    {
        // we have buffI and buffOff correct


        // check if current buffer has available spot already
        if(currLine->buffer[buffOff] == UNUSEDINT)
        {
            currLine->buffer[buffOff] = c;
        }
        else if(buffOff > 0 && currLine->buffer[buffOff - 1] == UNUSEDINT)
        {
            // insert to the left
            currLine->buffer[buffOff - 1] = c;
            //fprintf(stderr, "REUSE BUFFER");
        }
        else
        {
            // make a new buffer
            // copy everything on or to the right to new buffer
            // fill in current buffer's right with UNUSEDINT and c

            //fprintf(stderr, "NEW BUFFER");

            Buffer* newbuf = new Buffer();
            for (int i = buffOff, ii = 0; i < BUFFERLEN; i++, ii++)
            {
                //fprintf(stderr, "added %c from %d->%d\n", currLine->buffer[i], i, ii);
                newbuf->buffer[ii] = currLine->buffer[i];
            }

            //fprintf(stderr, "memset from %d for len %d\n", buffOff, BUFFERLEN - buffOff);
            for( int i = buffOff; i < BUFFERLEN; i++)
                currLine->buffer[i] = UNUSEDINT;
            //memset(currLine->buffer + buffOff, UNUSEDINT, BUFFERLEN - buffOff);
            currLine->buffer[buffOff] = c;
            newbuf->next = currLine->next;
            currLine->next = newbuf;

            Buffer* p = line->line;
            while(p != nullptr)
            {
                //fprintf(stderr, "%p, ", p);
                p = p->next;
            }
        }
    }
    






    //
    // for(int i = 0; i < buffI; i++)
    // {
    //         toinsert = toinsert->next;
    //
    //         if(toinsert == nullptr && i < buffI-1)
    //         {
    //             fprintf(stderr, "[%s] bad index %lu\n", __func__, index);
    //             return;
    //         }
    //         else if(toinsert == nullptr)
    //         {
    //             // allocate a new buffer
    //             p->next = new Buffer();
    //             toinsert = p->next;
    //             line->lineCapacity += BUFFERLEN;
    //         }
    //
    //         p = toinsert;
    // }
    //
    //
    // line->lineLength++;
    //
    // // check if current buffer has available spot already
    // if(buffI > 0 && toinsert->buffer[buffOff - 1] == UNUSEDINT)
    // {
    //     // insert to the left
    //     toinsert->buffer[buffOff - 1] = c;
    //     //fprintf(stderr, "REUSE BUFFER");
    // }
    // else
    // {
    //     // make a new buffer
    //     // copy everything on or to the right to new buffer
    //     // fill in current buffer's right with UNUSEDINT and c
    //
    //     //fprintf(stderr, "NEW BUFFER");
    //
    //     Buffer* newbuf = new Buffer();
    //     for (int i = buffOff, ii = 0; i < CHARBUFFER; i++, ii++)
    //     {
    //         newbuf->buffer[ii] = toinsert->buffer[i];
    //     }
    //     memset(toinsert->buffer + buffOff, UNUSEDINT, CHARBUFFER - buffOff);
    //     toinsert->buffer[buffOff] = c;
    //     newbuf->next = toinsert->next;
    //     toinsert->next = newbuf;
    // }
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

    newline->next = currentLine->next;
    currentLine->next=newline;
}

//returns false if it doesnt exist and sets currentline to before null
    // and true if it does if current line is null that means head does not exist

bool LockFreeList::getLine(size_t line, BufferList*& currentLine)
{
    BufferList* traverser=head;
    if (traverser==nullptr) {
        const char* c = __func__;
        log("[%s] Head did not exist", c );
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

    BufferList* buf;
    bool ret = getLine(line, buf);

    if(ret)
    {
        size_t width = 0;

        Buffer * b = buf->line;

        while(b != nullptr)
        {
            for(int i = 0; i < CHARBUFFER; i++)
                if(b->buffer[i] != UNUSEDINT)
                    width++;

            b = b->next;
        }

        return width;
    }
    else
    {
        const char* c = __func__;
        log( "[%s] Bad line %lu ", c, line);

        return 0;
    }
}



void LockFreeList::remove(size_t line, size_t index){
    //std::cout<<" remove";
    BufferList* removePoint;

    if(index == (size_t)-1)
    {
        if(line == 0)
            return;

        if(line_width(line) > 0)
        {
            bool ret = getLine(line, removePoint);
            BufferList* before;
            getLine(line-1, before);

            // copy over the chars
            size_t in = line_width(line-1);

            Buffer* t = removePoint->line;

            while(t)
            {
                for(int i = 0; i < BUFFERLEN; i++)
                {
                    if(t->buffer[i] != UNUSEDINT)
                    {
                        insertIntoLine(before, in, t->buffer[i]);
                        in++;
                    }
                }

                t = t->next;
            }

            before->next = removePoint->next;
        }
        else
        {
            // just remove it
            bool ret = getLine(line, removePoint);
            BufferList* before;
            getLine(line -1, before);

            before->next = removePoint->next;
        }
    }

    else
    {
        bool ret = getLine(line, removePoint);
        if(!ret ) {
            const char* c = __func__;
            log("[!!] Bad line of %lu (max len: %lu) in %s", line, dataLength * CHARBUFFER, c);
            return ;
        }
        //std::cout<<"should remove";
        Buffer * currLine=removePoint->line;
        int currIndex=0;
        bool removed = false;
        while(currLine!=NULL)
        {
            for (size_t i = 0; i < BUFFERLEN; i++) 
            {
                if(currLine->buffer[i]!=UNUSEDINT) 
                {
                    if (currIndex==index) 
                    {
                        currLine->buffer[i]=UNUSEDINT;
                        removed = true;
                        break;
                    }
                    currIndex++;
                }

                if(removed) break;
            }
            if (removed) 
                break;
            currLine=currLine->next;
        }

    }
}
void LockFreeList::move(size_t line, size_t index){
    // update the line and index object variables
    // insert a buffer there

}

void LockFreeList::print(WINDOW* win, size_t line,size_t maxWidth, size_t maxHeight){

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
    BufferList * list;
    bool ret = getLine(line, list);

    if(!ret)
    {
        const char* c = __func__;
        log("%s bad line index %lu", c, line);
        return;
    }

    wclear(win);
    size_t index = 0;
    std::vector<int> chars;

    while(list!=nullptr){
        if(index >= maxHeight)
            break;

        //wmove(win, index, 0);
        //waddstr(mainWindow, "                                                                                                                                    ");

        Buffer* lineData=list->line;
        size_t printindex = 0;
        while(lineData!=nullptr){
            for (size_t i = 0; i < CHARBUFFER; i++) {
                if (lineData->buffer[i]!=UNUSEDINT) {
                     //mvwaddch(win, index, printindex, lineData->buffer[i]);
                    chars.push_back(lineData->buffer[i]);

                    printindex++;
                }

            }
            lineData=lineData->next;
        }

        chars.push_back(ENTER_KEY);
        list=list->next;
        index++;
    }


    std::string s (chars.begin(), chars.end());
    printColored(win, s);
}

bool LockFreeList::contains(std::vector<std::pair<size_t, size_t>> & v, size_t s)
{
    for(auto & r : v)
    {
        if (s >= r.first && s < r.first + r.second)
            return true;
    }

    return false;
}

void LockFreeList::printColored(WINDOW* win, std::string text)
{

    //std::lock_guard<std::mutex> lock(vectorLock);
    // set up colors
    start_color();
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    init_pair(2, COLOR_CYAN, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    init_pair(4, COLOR_RED, COLOR_BLACK);

    std::vector<int> ctext(text.begin(), text.end());

    // loop over regex matches
    std::vector<std::pair<std::string, int>> keywords = {std::make_pair("\"[^\"]*\"", 4), std::make_pair("\\bfor\\b", 3), std::make_pair("\\bwhile\\b", 3), std::make_pair("\\bdo\\b", 3), std::make_pair("\\breturn\\b", 3), std::make_pair("\\bswitch\\b", 3), std::make_pair("\\belse\\b", 3), std::make_pair("\\bif\\b", 3), std::make_pair("\\bbreak\\b", 3), std::make_pair("\\bnamespace\\b", 3), std::make_pair("[\\d]+", 1), std::make_pair("#[\\w]+", 3)};

    std::string types[] = {"int", "long", "string", "char", "size_t", "ssize_t", "short", "bool", "[\\w]+_t"};

    std::vector<std::pair<size_t, size_t>> already_matched;

    for (std::string type : types)
    {
        char* ftype;
        asprintf(&ftype, "\\b%s\\b", type.c_str());
        keywords.push_back(std::make_pair(std::string(ftype), 2));
        free(ftype);
    }

    bool any = false;
    for(auto r : keywords)
    {
        //log("started matchingn");
        std::smatch m;
        try
        {
            std::string temptext = text;
            size_t offset = 0;
            while(std::regex_search(temptext, m, std::regex(r.first)))
            {
                any = true;

                // got a match
                for(int i = 0; i < m.size(); i++)
                {
                    // i is the index into m of our match
                    // m.position(i) is index into text that the match starts at
                    // m.length(i) is length of the match

                    //log("got match '%s' at index %d, pos %ld and len %ldn", m[i].str().c_str(), i, m.position(i), m.length(i));

                    std::string trimmed = m[i].str();
                    trim(trimmed);

                    bool colored = false;
                    size_t first = 0;
                    for (int po = m.position(i), le = m.length(i), in = po; in < po + le; in++)
                    {
                        if(!std::isspace(ctext[in + offset]) && !contains(already_matched, in + offset))
                        {
                            ctext[in + offset] |= COLOR_PAIR(r.second);
                            if(!colored)
                                first = in + offset;
                            colored = true;
                        }
                    }

                    if(colored)
                        already_matched.push_back(std::make_pair(first, trimmed.length()));
                }

                temptext = m.suffix().str();
                offset += m.length(0) + m.position(0);
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }

        //log("done matching");
    }

    if(any);
        //log("");

    // print everything
    size_t y = 0;
    size_t x = 0;
    for(int ch : ctext)
    {
        if((char)ch == ENTER_KEY)
        {
            y++;
            x = 0;
        }
        else
        {
            mvwaddch(win, y, x, ch);
            x++;
        }
    }

}

/*debugging utilties*/
void LockFreeList::printDebug(){

}

void LockFreeList::writeToFileDebug(){

}


void LockFreeList::writeToFile(std::string filename)
{
    // serialize ourselves to a file
    // could do realloc but too lazy, so do a two pass

    std::ofstream outFile;
    outFile.open(filename, std::fstream::out);

    BufferList * list;
    bool ret = getLine(0, list);

    while(list!=nullptr){

        Buffer* lineData=list->line;
        size_t printindex = 0;
        while(lineData!=nullptr){
            for (size_t i = 0; i < CHARBUFFER; i++) {
                if (lineData->buffer[i]!=UNUSEDINT) {
                    outFile << (char)lineData->buffer[i];
                }

            }
            lineData=lineData->next;
            outFile << std::endl;
        }

        list=list->next;
    }

}


size_t LockFreeList::readFromFile(std::string filename)
{
    clear();


    std::fstream in;
    in.open(filename, std::fstream::in);

    size_t linei = 0;
    size_t i = 0;
    std::string line;

    while(std::getline(in, line))
    {
        i = 0;

        for(const char & c : line)
        {
            if(c != ENTER_KEY && c != '\n')
                insert(linei, i++, c);
        }
        insert(linei, i++, ENTER_KEY);
        linei++;
    }

    return linei;
}

std::pair<char*, size_t> LockFreeList::serialize()
{

    // serialize ourselves to a new buffer
    // could do realloc but too lazy, so do a two pass

    // add 12 bytes (3 ints) at the beginning, for reasons
    size_t len = 3;

    //for(size_t y = 0; y < data.size(); y++)
    //{
        //for(size_t x = 0; x < data[y].size(); x++)
        //{
            //len ++;
        //}
        //len ++;     // add in the newline terminator
    //}

    BufferList * list;
    getLine(0, list);

    while(list!=nullptr){

        Buffer* lineData=list->line;
        size_t printindex = 0;
        while(lineData!=nullptr){
            for (size_t i = 0; i < CHARBUFFER; i++) {
                if (lineData->buffer[i]!=UNUSEDINT) {
                    len++;
                }

            }
            lineData=lineData->next;
            len++;      // add in the newline
        }

        list=list->next;
    }

    int* ret = new int[len];
    memset(ret, 0, sizeof(int)*3);

    // start at 4th int
    size_t i = 3;

    //for(size_t y = 0; y < data.size(); y++)
    //{
        //for(size_t x = 0; x < data[y].size(); x++)
        //{
            //ret[i++] = htonl(data[y][x]);
        //}

        //ret[i++] = htonl((int)'\n');     // add in the newline terminator
    //}

    getLine(0, list);

    while(list!=nullptr){

        Buffer* lineData=list->line;
        size_t printindex = 0;
        while(lineData!=nullptr){
            for (size_t i = 0; i < CHARBUFFER; i++) {
                if (lineData->buffer[i]!=UNUSEDINT) {
                    ret[i++] = htonl(lineData->buffer[i]);
                }
            }
            lineData=lineData->next;
            ret[i++] = htonl(ENTER_KEY);
        }

        list=list->next;
    }


    return std::make_pair((char*)ret, (len)*4);
}


size_t LockFreeList::deserialize(char* ibuf, size_t len)
{
    // get a reasonable int array back
    // also skip first 12 bytes (3 ints) for reasons
    int* buf = (int*)(ibuf + 12);
    len /= 4;
    len -= 3;

    // clear us
    clear();

    size_t line = 0;
    size_t read = 0;
    size_t i = 0;
    // throw away first 10 bytes, for reasons
    int* t = buf;

    //log("deserializing %lu chars", len);

    while(read < len)
    {
        i = 0;

        while(ntohl(*t) != ENTER_KEY)
        {
            insert(line, i++, ntohl(*t));
            read++;
            t++;
            //log("here2");
        }
        insert(line, i++, ENTER_KEY);
        line++;
        read++;
        t++;
        //log("here1");
        // add a new line 
    }

    return line;
}

void LockFreeList::clear()
{

    BufferList * list;
    BufferList* nlist;
    bool ret = getLine(0, list);

    while(list!=nullptr)
    {
        nlist = list->next;

        Buffer* lineData=list->line;
        Buffer* next = nullptr;
        size_t printindex = 0;
        while(lineData!=nullptr)
        {
            next = lineData->next;

            delete lineData;

            lineData = next;
        }

        delete list;

        list = nlist;
    }

    head=new BufferList();

    head->line= new Buffer();
    head->lineCapacity+=BUFFERLEN;
    head->lineLength=0;
    dataLength = 1;
}

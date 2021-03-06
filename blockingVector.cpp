#include "blockingVector.h"

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
        log("[!!] bad line access! %lu >= %lun", line, data.size());
        return;
    }

    if(index > data[line].size())
    {
        log("[!!] bad index access! %lu > %lun", index, data[line].size());
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
        log("delete line idx: %lu", line);

        if(line!=0) {
            if(line >= data.size())
            {
                log("[!!] bad line access! %lu >= %lun", line, data.size());
                return;
            }

            if( data[line].size() > 0)
            {
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
            log("[!!] bad line access! %lu >= %lun", line, data.size());
            return;
        }

        if(index > data[line].size())
        {
            log("[!!] bad index access! %lu > %lun", index, data[line].size());
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
            if(c != '\n' && c != ENTER_KEY)
                newvec[vecindex].push_back(c);
            //else
             //   log("newline: %lu", vecindex);
        }
        vecindex++;
        newvec.push_back(std::vector<int>());
    }
    // the last line we don't need, it didn't actually exist
    newvec.pop_back();
   // log("newvecsize: %lu", newvec.size());
    data = std::move(newvec);


    return data.size();
}

void BlockingVector::print(WINDOW* win, size_t line,size_t maxWidth, size_t maxHeight){
    std::lock_guard<std::mutex> lock(vectorLock);

    size_t index = 0;
    std::vector<int> chars;
    std::unordered_map<size_t, bool> nls;

    wclear(win);

    for(size_t i = line; i < std::min(this->data.size(), line + maxHeight); i++, index++){
        // clear screen
        //wmove(win, index, 0);
        //waddstr(win, "                                                                                                                                                 ");   // clear line
        for(size_t j = 0; j < std::min(this->data[i].size(), maxWidth); j++){
            //std::cout << (char)data[i][j];
            //mvwaddch(win, index, j, this->data[i][j]);
            chars.push_back(this->data[i][j]);
        }
        //std::cout<<std::endl;
        chars.push_back(ENTER_KEY);
        nls[chars.size() - 1] = true;
    }

    std::string s (chars.begin(), chars.end());
    printColored(win, s, nls);
}

bool BlockingVector::contains(std::vector<std::pair<size_t, size_t>> & v, size_t s)
{
    for(auto & r : v)
    {
        if (s >= r.first && s < r.first + r.second)
            return true;
    }

    return false;
}

void BlockingVector::printColored(WINDOW* win, const std::string& text, std::unordered_map<size_t, bool> & nls)
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

    std::string types[] = {"int", "long", "string", "char", "size_t", "ssize_t", "short", "bool", "void", "struct", "class", "[\\w]+_t"};

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
    size_t index = 0;
    for(int ch : ctext)
    {
        if(nls[index])
        {
            //log("y: %lu, index: %lu", y, index);
            y++;
            x = 0;

        }
        else
        {
            mvwaddch(win, y, x, ch);
            x++;
        }

        index++;
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

        ret[i++] = htonl(ENTER_KEY);     // add in the newline terminator
    }

    size_t count = 0;
    for (i = 0; i < len; i++)
        count += (ret[i] == htonl(ENTER_KEY) ? 1 : 0);

    //log("sending %lu lines, but %lu newlines", data.size(), count);

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
        while(ntohl(*t) != ENTER_KEY)
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

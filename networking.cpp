#include "networking.h"

std::mutex sendlock;

volatile bool SHUTDOWN_NETWORK;

//https://stackoverflow.com/questions/3022552/is-there-any-standard-htonl-like-function-for-64-bits-integers-in-c
uint64_t htonll(uint64_t value)
{
    // The answer is 42
    static const int num = 42;

    // Check the endianness
    if (*reinterpret_cast<const char*>(&num) == num)
    {
        const uint32_t high_part = htonl(static_cast<uint32_t>(value >> 32));
        const uint32_t low_part = htonl(static_cast<uint32_t>(value & 0xFFFFFFFFLL));

        return (static_cast<uint64_t>(low_part) << 32) | high_part;
    } else
    {
        return value;
    }
}

uint64_t ntohll(uint64_t value)
{
    // The answer is 42
    static const int num = 42;

    // Check the endianness
    if (*reinterpret_cast<const char*>(&num) == num)
    {
        // if we are little, we need to swap
        const uint32_t high_part = ntohl(static_cast<uint32_t>(value >> 32));
        const uint32_t low_part = ntohl(static_cast<uint32_t>(value & 0xFFFFFFFFLL));

        return (static_cast<uint64_t>(low_part) << 32) | high_part;
    } 
    else
    {
        // if we are big, no need
        return value;
    }
}

// will perform one pass and see if the footer could exist in this part of the buffer or later
bool footer_could_exist(char* buf)
{
    bool chain = 0;
    int same = 0;

    for(int i = 0; i < HEADER_LENGTH && same < HEADER_LENGTH; i++)
    {
        if(buf[i] == MESSAGE_FOOTER[same])
        {
            same++;
            chain = 1;
        }
        else
        {
            chain = 0;
            same = 0;
        }
    }

    return chain;
}

// looks for the footer and returns both whether it was found and also at what position in the buffer it STARTS
std::pair<bool, size_t> footer_exists(char* buf, size_t len)
{
    int same = 0;

    size_t i;
    for(i = 0; i < len && same < HEADER_LENGTH; i++)
    {
        if(buf[i] == MESSAGE_FOOTER[same])
        {
            same++;
        }
        else
        {
            // not same, aka not same chain
            same = 0;

            // check again, as it could be the start of a new chain
            if(buf[i] == MESSAGE_FOOTER[0])
            {
                same++;
            }
        }
    }

    // ugh, oh well the ternary has been written
    return std::make_pair((same == HEADER_LENGTH ? true : false), i);
}

std::pair<char*,size_t> get_message(int sock, bool block)
{

    static char garbage[MTU];
    if(!block)
    {
        ssize_t a = recv(sock, garbage, MTU, MSG_DONTWAIT | MSG_PEEK);

        if(a <= 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR))
            return std::make_pair(nullptr, 0);
        else if (a <= 0)
        {
            perror("get_message peek");
            return std::make_pair(nullptr, 1);
        }
    }

    // get a buffer the maximum size of a message
    char* tbuf = (char*)calloc(1, MTU + 1);
    char* retbuf = (char*)calloc(1, MTU + 1);

    size_t total_got = 0;

    while(1)
    {
        // peek so that we can only read what we need to get the footer
        ssize_t got = recv(sock, tbuf, MTU, MSG_PEEK);

        if(SHUTDOWN_NETWORK)
            return std::make_pair(nullptr, 0);

        //fprintf(stderr, "got: %ld\n", got);

        if(got == 0)
        {
            // they shutdown
            free(retbuf);
            free(tbuf);

            return std::make_pair(nullptr, -1);
        }
        
        if(got < 0)
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
            {
                char e[1024];
                strerror_r(errno, e, 1024);
                fprintf(stderr, "[%s] Retrying recv: %s\n", __func__, e);
                continue;
            }

            size_t r = 0;
            free(retbuf);
            free(tbuf);
            if(got != 0)
            {
                perror("[get_message] recv");
                r = 1;
            }
            return std::make_pair(nullptr, r);
        }

        if(got < 10 )
        {
            fprintf(stderr, "[!] Got short message!!! %ld (%lu)\n", got, total_got);
            usleep(1000);
            continue;
        }

        if(total_got == 0)
        {
            // verify we are getting a message header

            static char t[HEADER_LENGTH + 1];
            memcpy(t, MESSAGE_HEADER, HEADER_LENGTH);
            t[HEADER_LENGTH] = 0;

            if(strncmp(t, tbuf, HEADER_LENGTH) != 0)
            {
                // we didn't read a header
                fprintf(stderr, "[!!!] Bad header read! Got '%s' expected '%s'\n", tbuf, t);
                //fprintf(stderr, "[!!!] Bad header read! Got '%d' expected '%d'\n", tbuf[0], t[0]);

                return std::make_pair(nullptr, 1);
            }
        } 

        retbuf = (char*)realloc(retbuf, total_got + got);
        memcpy(retbuf + total_got, tbuf, got);

        // scan for the footer

        // this here assumes we never read less than HEADER_LENGTH
        ssize_t off = std::max((ssize_t)total_got - HEADER_LENGTH, 0L);
        size_t len = got + (total_got > HEADER_LENGTH ? HEADER_LENGTH : 0);
        // TODO: fix this line VVV
        std::pair<bool, size_t> ret = footer_exists(retbuf + off, len);

        if(ret.first)
        {
            // footer found
            // this should probably be happening almost everytime

            // number from start + length of footer + length of start to new stuff
            //size_t actually_read = ret.second + HEADER_LENGTH + (total_got > HEADER_LENGTH ? HEADER_LENGTH : total_got);
            //ssize_t actually_read = ret.second - (total_got > HEADER_LENGTH ? HEADER_LENGTH : 0);
            ssize_t actually_read = ret.second - (off < total_got ? HEADER_LENGTH : 0);

            // now remove those bytes from the read queue, this should leave the next header right there to read
            recv(sock, garbage, actually_read, 0);

            //retbuf = (char*)realloc(retbuf, total_got + actually_read);
            //memcpy(retbuf + total_got, tbuf, actually_read);

            if(total_got + actually_read - 2 * HEADER_LENGTH == 32)
            {
                int x = 0;
                x = 1;
            }

            free(tbuf);

            return std::make_pair(retbuf + HEADER_LENGTH, total_got + actually_read - 2 * HEADER_LENGTH);
        }
        // else keep going

        //retbuf = (char*)realloc(retbuf, total_got + got);
        //memcpy(retbuf + total_got, tbuf, got);

        total_got += got;
        
        // now remove those bytes from the read queue, this should leave the next header right there to read
        recv(sock, garbage, got, 0);

        //fprintf(stderr, "Reading more data\n");

    }

    free(tbuf);

    fprintf(stderr, "[!] Exited loop in get_message\n");

    return std::make_pair(retbuf + HEADER_LENGTH, total_got - HEADER_LENGTH);
}


void free_message(char* msg)
{
    free(msg - HEADER_LENGTH);
}

bool send_message(int sock, char* buf, size_t num_bytes)
{
    std::lock_guard<std::mutex> m(sendlock);

    int yes = 1, no = 0;

    /*
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void*)&no, sizeof(int));
    setsockopt(sock, IPPROTO_TCP, TCP_CORK, (void*)&yes, sizeof(int));
    */

    size_t left = num_bytes;
    size_t total_sent = 0;
    int mtu = MTU;

    //send(sock, MESSAGE_HEADER, HEADER_LENGTH, MSG_MORE);
    send(sock, MESSAGE_HEADER, HEADER_LENGTH, 0);

    while(left > 0)
    {
        //int flags = (left > mtu ? MSG_MORE : 0);
        int flags = (MSG_MORE);     // due to header/footer we need to add more every time
        flags = 0;
        ssize_t sent = send(sock, buf + total_sent, std::min(left, (size_t)mtu), flags);


        if(sent == -1)
        {
            //errors

            if(errno == EMSGSIZE)
            {
                mtu = (int)(mtu*0.9);
            }
            else if (errno == EAGAIN || errno == EINTR)
                continue;
            else 
            {
                perror("[send_message] send");
                return false;
            }
        }
        else
        {
            if(sent < 10)
            {
                fprintf(stderr, "[!] Sent short message!\n");
            }
            total_sent += sent;
            left -= sent;
        }
    }

    send(sock, MESSAGE_FOOTER, HEADER_LENGTH, 0);

    /*
    setsockopt(sock, IPPROTO_TCP, TCP_CORK, (void*)&no, sizeof(int));
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void*)&yes, sizeof(int));
    */

    return true;
}

bool send_move(int sock, size_t y, size_t x)
{
    size_t len = sizeof(short) + 2 * sizeof(size_t);
    char* buf = new char[len]();

    *(short*)buf = htons((short)PacketType::Move);
    *(size_t*)(buf + sizeof(short)) = htonll(y);
    *(size_t*)(buf + sizeof(short) + sizeof(size_t)) = htonll(x);

    bool ret = send_message(sock, buf, len);

    if(!ret)
    {
        fprintf(stderr, "[!!!] [%s] Bad return value\n", __func__);
    }

    delete [] buf;
}


bool send_insert(int sock, size_t y, size_t x, int c)
{
    size_t len = sizeof(short) + sizeof(int) + 2 * sizeof(size_t);
    char* buf = new char[len]();

    *(short*)buf = htons((short)PacketType::Insert);
    *(size_t*)(buf + sizeof(short)) = htonll(y);
    *(size_t*)(buf + sizeof(short) + sizeof(size_t)) = htonll(x);
    *(int*)(buf + sizeof(short) + 2*sizeof(size_t)) = htonl(c);

    bool ret = send_message(sock, buf, len);

    if(!ret)
    {
        fprintf(stderr, "[!!!] [%s] Bad return value\n", __func__);
    }

    delete [] buf;

    return ret;
}


bool send_remove(int sock, size_t y, size_t x)
{
    size_t len = sizeof(short) + 2 * sizeof(size_t);
    char* buf = new char[len]();

    *(short*)buf = htons((short)PacketType::Remove);
    *(size_t*)(buf + sizeof(short)) = htonll(y);
    *(size_t*)(buf + sizeof(short) + sizeof(size_t)) = htonll(x);

    bool ret = send_message(sock, buf, len);

    if(!ret)
    {
        fprintf(stderr, "[!!!] [%s] Bad return value\n", __func__);
    }

    delete [] buf;

    return ret;
}

bool send_write(int sock, std::string filename)
{
    size_t len = sizeof(short) + filename.length() + 1;
    len = std::max((size_t)10, len);// for reasons, make this at least 10
    char* buf = new char[len]();

    *(short*)buf = htons((short)PacketType::WriteToDisk);
    memcpy(buf + sizeof(short), filename.c_str(), filename.length() + 1);

    bool ret = send_message(sock, buf, len);

    if(!ret)
        fprintf(stderr, "[!!!] [%s] bad return value\n", __func__);

    delete [] buf;

    return ret;
}

bool send_write_confirm(int sock, std::string filename)
{
    fprintf(stderr, "%s\n", __func__);
    size_t len = sizeof(short) + filename.length() + 1;
    len = std::max((size_t)10, len);// for reasons, make this at least 10
    char* buf = new char[len]();

    *(short*)buf = htons((short)PacketType::WriteConfirmed);
    memcpy(buf + sizeof(short), filename.c_str(), filename.length() + 1);

    bool ret = send_message(sock, buf, len);

    if(!ret)
        fprintf(stderr, "[!!!] [%s] bad return value\n", __func__);

    delete [] buf;

    return ret;
}

bool broadcast_write_confirm(std::vector<int> sockets, std::string filename)
{
    size_t len = sizeof(short) + filename.length() + 1;
    len = std::max((size_t)10, len);// for reasons, make this at least 10
    char* buf = new char[len]();

    *(short*)buf = htons((short)PacketType::WriteConfirmed);
    memcpy(buf + sizeof(short), filename.c_str(), filename.length()+1);

    for(int sock : sockets)
    {
        bool ret = send_message(sock, buf, len);

        if(!ret)
            fprintf(stderr, "[!!!] [%s] bad return value for client with sock %d\n", __func__, sock);
    }

    delete [] buf;

    return true;
}

bool send_read(int sock, std::string filename)
{
    size_t len = sizeof(short) + filename.length() + 1;
    len = std::max((size_t)10, len);// for reasons, make this at least 10
    char* buf = new char[len]();

    *(short*)buf = htons((short)PacketType::ReadFromDisk);
    memcpy(buf + sizeof(short), filename.c_str(), filename.length()+1);

    bool ret = send_message(sock, buf, len);

    if(!ret)
        fprintf(stderr, "[!!!] [%s] bad return value\n", __func__);

    delete [] buf;

    return ret;
}

bool send_read_confirm(int sock, size_t lines, std::string filename)
{
    size_t len = sizeof(short) + sizeof(size_t) + filename.length() + 1;
    len = std::max((size_t)10, len);// for reasons, make this at least 10
    char* buf = new char[len]();

    *(short*)buf = htons((short)PacketType::ReadConfirmed);
    *(size_t*)(buf + sizeof(short)) = htonll(lines);
    memcpy(buf + sizeof(short) + sizeof(size_t), filename.c_str(), filename.length()+1);

    bool ret = send_message(sock, buf, len);

    if(!ret)
        fprintf(stderr, "[!!!] [%s] bad return value\n", __func__);

    delete [] buf;

    return ret;
}

bool broadcast_read_confirm(const std::vector<int> & sockets, size_t lines, std::string filename)
{
    size_t len = sizeof(short) + sizeof(size_t) + filename.length() + 1;
    len = std::max((size_t)10, len);// for reasons, make this at least 10
    char* buf = new char[len]();

    *(short*)buf = htons((short)PacketType::ReadConfirmed);
    *(size_t*)(buf + sizeof(short)) = htonll(lines);
    memcpy(buf + sizeof(short) + sizeof(size_t), filename.c_str(), filename.length()+1);

    for(int sock : sockets)
    {
        bool ret = send_message(sock, buf, len);

        if(!ret)
            fprintf(stderr, "[!!!] [%s] bad return value for client with sock %d\n", __func__, sock);
    }

    delete [] buf;

    return true;
}

bool send_get_full(int sock)
{
    fprintf(stderr, "[%s] entered\n", __func__);
    
    size_t len = sizeof(short);
    len = std::max((size_t)10, len);// for reasons, make this at least 10
    char* buf = new char[len]();

    *(short*)buf = htons((short)PacketType::GetFull);

    bool ret = send_message(sock, buf, len);

    if(!ret)
        fprintf(stderr, "[!!!] [%s] bad return value\n", __func__);

    delete [] buf;

    fprintf(stderr, "Sent a getfull\n");

    return ret;

}

bool broadcast_disconnect(const std::vector<int>& sockets)
{
    std::string msg = "Client disconnected!";

    size_t len = sizeof(short) + msg.length() + 1;
    len = std::max((size_t)10, len);// for reasons, make this at least 10
    char* buf = new char[len]();

    *(short*)buf = htons((short)PacketType::Disconnect);
    memcpy(buf + sizeof(short), msg.c_str(), msg.length()+1);

    for(int sock : sockets)
    {
        bool ret = send_message(sock, buf, len);

        if(!ret)
            fprintf(stderr, "[!!!] [%s] bad return value for client with sock %d\n", __func__, sock);
    }

    delete [] buf;

    return true;
}

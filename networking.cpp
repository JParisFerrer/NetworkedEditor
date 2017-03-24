#include "networking.h"

// will perform one pass and see if the footer could exist in this part of the buffer or later
bool footer_could_exist(char* buf)
{
    bool chain = 0;
    int same = 0;

    for(int i = 0; i < HEADER_LENGTH, same < HEADER_LENGTH; i++)
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
    for(i = 0; i < len, same < HEADER_LENGTH; i++)
    {
        if(buf[i] == MESSAGE_FOOTER[same])
        {
            same++;
        }
        else
        {
            // not same, aka not same chain
            same = 0;
        }

        // check again, as it could be the start of a new chain
        if(buf[i] == MESSAGE_FOOTER[0])
        {
            same++;
        }
    }

    return std::make_pair((same == HEADER_LENGTH ? true : false), i);
}

std::pair<char*,size_t> get_message(int sock)
{
    // get a buffer the maximum size of a message
    char* tbuf = (char*)calloc(1, MTU + 1);
    char* retbuf = (char*)calloc(1, MTU + 1);

    size_t total_got = 0;

    while(1)
    {
        // peek so that we can only read what we need to get the footer
        ssize_t got = recv(sock, tbuf, MTU, MSG_PEEK );

        if(total_got == 0)
        {
            // verify we are getting a message header

            static char t[HEADER_LENGTH + 1];
            strncpy(t, MESSAGE_HEADER, HEADER_LENGTH);
            t[HEADER_LENGTH] = 0;

            // because we calloced this should be fine
            if(strcmp(t, tbuf) != 0)
            {
                // we didn't read a header
                fprintf(stderr, "[!!!] Bad header read! Got '%s' expected '%s'\n", tbuf, t);
            }
        }

        if(got < 0)
        {
            free(retbuf);
            free(tbuf);
            perror("[get_message] recv");
            return std::make_pair(nullptr, 0);
        }

        // scan for the footer
        ssize_t off = std::max((ssize_t)total_got - HEADER_LENGTH, 0L);
        size_t len = got + (total_got > HEADER_LENGTH ? HEADER_LENGTH : total_got);
        std::pair<bool, size_t> ret = footer_exists(retbuf + off, len);

        if(ret.first)
        {
            // footer found
            // this should probably be happening almost everytime

            // number from start + length of footer + length of start to new stuff
            size_t actually_read = ret.second + HEADER_LENGTH + (total_got > HEADER_LENGTH ? HEADER_LENGTH : total_got);

            // now remove those bytes from the read queue, this should leave the next header right there to read
            recv(sock, nullptr, actually_read, 0);

            retbuf = (char*)realloc(retbuf, total_got + actually_read);
            memcpy(retbuf + total_got, tbuf, actually_read);

            free(tbuf);

            return std::make_pair(retbuf, total_got + actually_read);
        }
        // else keep going

        retbuf = (char*)realloc(retbuf, total_got + got);
        memcpy(retbuf + total_got, tbuf, got);

        total_got += got;
        
        // now remove those bytes from the read queue, this should leave the next header right there to read
        recv(sock, nullptr, got, 0);

    }

    free(tbuf);

    return std::make_pair(retbuf, total_got);
}

bool send_bytes(int sock, char* buf, size_t num_bytes)
{
    size_t left = num_bytes;
    size_t total_sent = 0;
    int mtu = MTU;

    send(sock, MESSAGE_HEADER, HEADER_LENGTH, MSG_MORE);

    while(left > 0)
    {
        //int flags = (left > mtu ? MSG_MORE : 0);
        int flags = (MSG_MORE);     // due to header/footer we need to add more every time
        ssize_t sent = send(sock, buf + total_sent, std::min(left, (size_t)mtu), flags);

        if(sent == -1)
        {
            //errors

            if(errno == EMSGSIZE)
            {
                mtu = (int)(mtu*0.9);
            }
            else 
            {
                perror("[send_bytes] send");
                return false;
            }
        }
        else
        {
            total_sent += sent;
            left -= sent;
        }
    }

    send(sock, MESSAGE_FOOTER, HEADER_LENGTH, 0);

    return true;
}

bool send_move(int sock, size_t y, size_t x)
{
    size_t len = sizeof(short) + 2 * sizeof(size_t);
    char* buf = new char[len];

    *(short*)buf = (short)PacketType::Move;
    *(size_t*)(buf + sizeof(short)) = y;
    *(size_t*)(buf + sizeof(short) + sizeof(size_t)) = x;

    bool ret = send_bytes(sock, buf, len);

    if(!ret)
    {
        fprintf(stderr, "[!!!] [%s] Bad return value\n", __func__);
    }
}

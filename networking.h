#ifndef NETWORKING_H
#define NETWORKING_H

#include <vector>
#include <cstdlib>
#include <algorithm>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <cstring>
#include <mutex>
#include <unistd.h>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <iostream>

#include "textcontainer.h"


#define MTU 1280

const char MESSAGE_HEADER[] = { 0, 1, 1, 1, 1, 1, 1, 1, 1, 0 };
const char MESSAGE_FOOTER[] = { 0, 4, 4, 4, 4, 4, 4, 4, 4, 0 };
const int HEADER_LENGTH = 10;

// this is a short, 65536 message types should be enough
enum class PacketType : short
{
    Debug = 0,
    Logging,
    Move,
    Insert,
    Remove,
    GetFull,
    FullContent,
    WriteToDisk,
    WriteConfirmed,
    ReadFromDisk,
    ReadConfirmed,
    Disconnect,
    GetClientCount,
    ClientCount,
    NewClient
};

static std::string PacketTypeNames[] = 
{
    "Debug",
    "Logging",
    "Move",
    "Insert",
    "Remove",
    "GetFull",
    "FullContent",
    "WriteToDisk",
    "WriteConfirmed",
    "ReadFromDisk",
    "ReadConfirmed",
    "Disconnect",
    "GetClientCount",
    "ClientCount",
    "NewClient"
};

static size_t PacketTypeNum = 15;

#define log(...) \
    do { \
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now(); \
    std::time_t now_c = std::chrono::system_clock::to_time_t(now); \
    char logcharbuf[4096]; \
    std::strftime(logcharbuf, 4095, "%H:%M:%S", std::localtime(&now_c)); \
    fprintf(stderr, "[%s] ", logcharbuf); \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, "\n"); \
    } while(0);

uint64_t htonll(uint64_t value);

uint64_t ntohll(uint64_t value);

std::pair<char*,size_t> get_message(int sock, bool block);

template <typename T>
T get_bytes_as(char* msg, size_t start_byte)
{
    T ret = *(T*)(msg + start_byte);


    if(sizeof(T) == 2)
    {
        //fprintf(stderr, "[%s] sizeof 2B\n", __func__);
        return (T)ntohs((uint16_t)ret);
    }
    if(sizeof(T) == 4)
    {
        //fprintf(stderr, "[%s] sizeof 4B\n", __func__);
        return (T)ntohl((uint32_t)ret);
    }
    if(sizeof(T) == 8)
    {
        //fprintf(stderr, "[%s] sizeof 8B\n", __func__);
        return (T)ntohll((uint64_t)ret);
    }

    //fprintf(stderr, "[%s] sizeof %luB\n", __func__, sizeof(T));
    return ret;
}

void free_message(char* msg);

// will loop and send all the bytes
bool send_message(int sock, char* buf, size_t num_bytes);

bool send_move(int sock, size_t y, size_t x);

bool send_insert(int sock, size_t y, size_t x, int c);

bool send_remove(int sock, size_t y, size_t x);

bool send_write(int sock, std::string filename);

bool send_write_confirm(int sock, std::string filename);
bool broadcast_write_confirm(std::vector<int> sockets, std::string filename);

bool send_read(int sock, std::string filename);

bool send_read_confirm(int sock, size_t lines, std::string filename);
bool broadcast_read_confirm(const std::vector<int>& sockets, size_t lines, std::string filename);

bool send_get_client_count(int sock);

bool send_client_count(int sock, int num_clients);

bool broadcast_new_client(const std::vector<int>& sockets);

bool send_get_full(int sock);

template <class T>
bool send_full_content(int sock, TextContainer<T>& text, bool forced)
{
    //log("sending full content");

    std::pair<char*, size_t> ser = text.serialize();

    size_t len = sizeof(short) + sizeof(char) + ser.second;
    char* buf = new char[len]();

    *(short*)buf = htons((short)PacketType::FullContent);
    *(char*)(buf + sizeof(short)) = (char)forced;
    memcpy(buf + sizeof(short) + 1, ser.first, ser.second);

    bool ret = send_message(sock, buf, len);

    if(!ret)
    {
        const char* c = __func__;
        log( "[!!!] [%s] bad return value", c);
    }

    free(ser.first);

    //log("sent full content");

    return ret;
}

template <class T>
bool broadcast_full_content(const std::vector<int>& sockets, TextContainer<T>& text, bool forced)
{
    //log("broadcasting full content");

    std::pair<char*, size_t> ser = text.serialize();

    size_t len = sizeof(short) + sizeof(char) + ser.second;
    char* buf = new char[len]();

    *(short*)buf = htons((short)PacketType::FullContent);
    *(char*)(buf + sizeof(short)) = (char)forced;
    memcpy(buf + sizeof(short) + 1, ser.first, ser.second);

    for(int sock : sockets)
    {

        bool ret = send_message(sock, buf, len);

        if(!ret)
        {
            const char* c = __func__;
            log("[!!!] [%s] bad return value for socket %d", c, sock);
        }

    }

    delete []ser.first;

    //log("broadcasted full content");

    return true;

}

bool broadcast_disconnect(const std::vector<int>& sockets);


#endif

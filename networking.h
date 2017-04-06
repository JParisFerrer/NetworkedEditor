#ifndef NETWORKING_H
#define NETWORKING_H

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
#include "textcontainer.h"

#define MTU 1280

const char MESSAGE_HEADER[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
const char MESSAGE_FOOTER[] = { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4 };
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
    ReadConfirmed
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
    "ReadConfirmed"
};

static size_t PacketTypeNum = 11;

std::pair<char*,size_t> get_message(int sock, bool block);

template <typename T>
T get_bytes_as(char* msg, size_t start_byte)
{
    return *(T*)(msg + start_byte);
}

void free_message(char* msg);

// will loop and send all the bytes
bool send_message(int sock, char* buf, size_t num_bytes);

bool send_move(int sock, size_t y, size_t x);

bool send_insert(int sock, size_t y, size_t x, int c);

bool send_remove(int sock, size_t y, size_t x);

bool send_write(int sock, std::string filename);

bool send_write_confirm(int sock, std::string filename);

bool send_read(int sock, std::string filename);

bool send_read_confirm(int sock, size_t lines, std::string filename);

bool send_get_full(int sock);

template <class T>
bool send_full_content(int sock, TextContainer<T>& text)
{
    std::pair<char*, size_t> ser = text.serialize();

    size_t len = sizeof(short) + ser.second + 1;
    char* buf = new char[len];

    *(short*)buf = (short)PacketType::FullContent;
    strncpy(buf + sizeof(short), ser.first, ser.second);

    bool ret = send_message(sock, buf, len);

    if(!ret)
        fprintf(stderr, "[!!!] [%s] bad return value\n", __func__);

    free(ser.first);

    return ret;
}

#endif

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
    WriteToDisk,
    WriteConfirmed,
    ReadFromDisk,
    ReadConfirmed
};


std::pair<char*,size_t> get_message(int sock);

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

bool send_write(int sock);

bool send_read(int sock, std::string filename);

#endif

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

#define MTU 1280

// this is a short, 65536 message types should be enough
enum class PacketType
{
    Debug = 0,
    Logging,
    Move,
    Insert,
    Remove,
    GetFull
};

// will loop and send all the bytes
bool send_bytes(int sock, char* buf, size_t num_bytes);

#endif

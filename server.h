#ifndef SERVER_H
#define SERVER_H

#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <cstdlib>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <vector>
#include <pthread.h>
#include <cstdio>
#include <string>
#include <iostream>
#include "networking.h"



namespace server
{

    int server_entrypoint();

}

#endif

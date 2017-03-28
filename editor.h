#ifndef EDITOR_H
#define EDITOR_H

#include <curses.h>
#include <iostream>
#include <sstream>
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
#include <cstdio>
#include "textcontainer.h"
#include "blockingVector.h"
#include "networking.h"


namespace client
{

    int client_entrypoint();

    void exit_handler(int sigint);
}

#endif

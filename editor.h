#ifndef EDITOR_H
#define EDITOR_H

#include "blockingVector.h"
#include "networking.h"
#include "textcontainer.h"
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <curses.h>
#include <errno.h>
#include <iostream>
#include <mutex>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <sstream>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

namespace client {

int client_entrypoint();

void exit_handler(int sigint);
} // namespace client

#endif

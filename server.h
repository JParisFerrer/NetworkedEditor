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
#include "textcontainer.h"
#include "blockingVector.h"
#include "lockfreelist.h"


namespace server
{

    class Client
    {
        private:
            static int nextid;

        public:
            int socket;
            bool alive;
            const int id;
            
            Client(int sock) : id(Client::nextid)
            {
                socket = sock;
                Client::nextid++;
                alive = true;
            }

    };


    int server_entrypoint();

}

#endif

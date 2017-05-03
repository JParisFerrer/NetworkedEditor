#ifndef SERVER_H
#define SERVER_H

#include "blockingVector.h"
#include "networking.h"
#include "textcontainer.h"
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <errno.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

namespace server {

class Client {
private:
  static int nextid;

public:
  int socket;
  bool alive;
  const int id;

  Client(int sock) : id(Client::nextid) {
    socket = sock;
    Client::nextid++;
    alive = true;
  }
};

int server_entrypoint();

} // namespace server

#endif

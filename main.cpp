#include "editor.h"
#include "server.h"
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <csignal>

bool START_SERVER = false;
std::string SERVER_ADDRESS = "127.0.0.1";
std::string SERVER_PORT = "29629";

pid_t SERVER_PID;

void parse_opts(int argc, char** argv)
{
    START_SERVER = true;
    SERVER_PORT = "29629";
    SERVER_ADDRESS = "127.0.0.1";
}

int main(int argc, char** argv)
{
    parse_opts(argc, argv);

    if(START_SERVER)
    {
        pid_t child = fork();

        if(child == -1)
        {
            std::perror("fork");
            std::exit(1);
        }
        else if (child == 0)
        {
            // child
            return server_entrypoint(SERVER_PORT);
        }
        else
        {
            // parent
            SERVER_PID = child;
        }
    }

    // pass in what the client should connect to
    // actually just use externs
    int ret = client_entrypoint();

    // if we started the server, kill it
    // it should handle SIG_TERM to cleanup, then quit
    if(START_SERVER)
        kill(SERVER_PID, SIGTERM);

    return ret;
}


#include "editor.h"
#include "server.h"
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <string>

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

            // let's redirect stderr to a file
            FILE* f = fopen("log.txt", "w");

            if(!f)
            {
                perror("server's fopen");
                return 10;
            }

            int fd = fileno(f);

            int ret = dup2(fd, fileno(stderr));

            if(ret)
            {
                perror("dup2");
                return 11;
            }

            return server::server_entrypoint();
        }
        else
        {
            // parent
            SERVER_PID = child;
        }
    }

    // pass in what the client should connect to
    // actually just use externs
    int ret = client::client_entrypoint();

    // if we started the server, kill it
    // it should handle SIG_TERM to cleanup, then quit
    if(START_SERVER)
        kill(SERVER_PID, SIGTERM);

    return ret;
}


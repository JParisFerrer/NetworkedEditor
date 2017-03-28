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

void main_int_handler(int arg)
{
    fprintf(stderr, "EXIT HANDLER\n");

    if(START_SERVER)
    {
        kill(SERVER_PID, SIGTERM);
    }

    client::exit_handler(0);

    // shouldn't reach here because that thing also has an exit(1)
    exit(1);
}

int main(int argc, char** argv)
{
    parse_opts(argc, argv);

    signal(SIGPIPE, SIG_IGN);

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
            FILE* f2 = fopen("logerr.txt", "w");

            if(!f || !f2)
            {
                perror("server's fopen");
                return 10;
            }

            int fd = fileno(f);
            int fd2 = fileno(f2);

            int ret = dup2(fd, fileno(stdout));
            int ret2 = dup2(fd2, fileno(stderr));

            if(ret == -1 || ret2 == -1)
            {
                perror("dup2");
                return 11;
            }

            int server_ret = server::server_entrypoint();

            fclose(f);
            fclose(f2);
            return server_ret;
        }
        else
        {
            // parent
            SERVER_PID = child;
        }
    }

    signal(SIGINT, main_int_handler);

    FILE* f = fopen("clogerr.txt", "w");
    if(!f)
    {
        perror("client's fopen");

        if(START_SERVER)
        {
            kill(SERVER_PID, SIGTERM);
        }

        return 20;
    }

    dup2(fileno(f), fileno(stderr));

    // pass in what the client should connect to
    // actually just use externs
    int ret = client::client_entrypoint();

    // if we started the server, kill it
    // it should handle SIG_TERM to cleanup, then quit
    if(START_SERVER)
    {
        kill(SERVER_PID, SIGTERM);
    }

    fclose(f);

    return ret;
}


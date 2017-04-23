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
std::string SERVER_SEARCH_PORT = "29629";

pid_t SERVER_PID;


/*

    Acceptable options:
        -a          address to connect to: string
        -sp          port to connect to: string
        -p         port to start server on: string
        --server    start the server, even if the -a flag exists

    if -a is set, don't start a server, unless --server is also specified

*/
void parse_opts(int argc, char** argv)
{
    std::vector<std::string> args;

    char** t = argv + 1;

    while(*t)
    {
        args.push_back(std::string(*t));
        t++;
    }

    if(args.empty())
    {
        START_SERVER = true;
        SERVER_PORT = "29629";
        SERVER_SEARCH_PORT = "29629";
        SERVER_ADDRESS = "127.0.0.1";
    }

    else
    {
        if(std::find(args.begin(), args.end(), "-p") != args.end())
        {
            auto it = (std::find(args.begin(), args.end(), "-p") + 1);
            if(it == args.end() || it->find("-") != std::string::npos)
                goto USAGE;
            SERVER_PORT = *it;
        }
        if(std::find(args.begin(), args.end(), "-sp") != args.end())
        {
            auto it = (std::find(args.begin(), args.end(), "-sp") + 1);
            if(it == args.end() || it->find("-") != std::string::npos)
                goto USAGE;

            SERVER_SEARCH_PORT = *it;
        }

        if(std::find(args.begin(), args.end(), "-a") != args.end())
        {
            // -a exists
            auto it = (std::find(args.begin(), args.end(), "-a") + 1);
            if(it == args.end() || it->find("-") != std::string::npos)
                goto USAGE;
            SERVER_ADDRESS = *it;

            if(std::find(args.begin(), args.end(),"--start") != args.end())
                START_SERVER = true;
            else
                START_SERVER = false;
        }
        else
        {
            // -a doesn't exist
            START_SERVER = true;
        }
    }

    fprintf(stdout, "Starting server: %s\n", std::to_string(START_SERVER).c_str());

    if(START_SERVER)
        fprintf(stdout, "Server port: %s\n", SERVER_PORT.c_str());

    fprintf(stdout, "Server search address: %s\n", SERVER_ADDRESS.c_str());
    fprintf(stdout, "Server search port: %s\n", SERVER_SEARCH_PORT.c_str());

    return;

USAGE:
    fprintf(stderr, "Usage: %s [-p PORT] [-a ADDRESS] [-sp SEARCH_PORT] [--start]\n", argv[0]);
    fprintf(stderr, "\n\tADDRESS: a string that is the address to attempt to connect to\n");
    fprintf(stderr, "\tPORT: a string that is the port to start the server on\n");
    fprintf(stderr, "\tSEARCH_PORT: a string that is the port to attempt to connect to\n");
    fprintf(stderr, "\t--start: whether to start the server when the -a flag is set\n");

    exit(1);
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

    sleep(1);

    // if we started the server, kill it
    // it should handle SIG_TERM to cleanup, then quit
    if(START_SERVER)
    {
        kill(SERVER_PID, SIGTERM);
    }

    fclose(f);

    return ret;
}


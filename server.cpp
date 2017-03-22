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

extern std::string SERVER_PORT;

// credit to https://beej.us/guide/bgnet/output/html/multipage/clientserver.html
int setup_network()
{
    struct addrinfo hints, *server_info, *trav;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int ret = getaddrinfo(NULL, SERVER_PORT, &hints, &server_info);

    if(ret)
    {
        fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(ret));
        return 1;
    }

    int socket_fd;
    int yes = 1;

    for(trav = servinfo; trav != NULL; trav = trav->ai_next) 
    {
        if ((socket_fd = socket(trav->ai_family, trav->ai_socktype, trav->ai_protocol)) == -1) 
        {
            perror("server: socket");
            continue;
        }

        if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) 
        {
            perror("setsockopt");
            return 2;
        }

        if (bind(socket_fd, trav->ai_addr, trav->ai_addrlen) == -1) 
        {
            close(socket_fd);
            perror("server: bind");
            continue;
        }

        // if we are here then our socket is good
        break;
    }

    freeaddrinfo(server_info);

    if(trav == NULL)
    {
        fprintf(stderr, "Server failed to bind\n");
        return 3;
    }

    if (listen(socket_fd, 10) == -1)
    {
        perror("listen");
        return 3;
    }
}

int setup()
{
    int ret = setup_network();

    if(ret)
        return ret;

    // other stuff

    return ret;
}

int server_entrypoint()
{
    setup();

    return 0;
}

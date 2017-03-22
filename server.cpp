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
#include <vector>

extern std::string SERVER_PORT;
int SERVER_SOCKET;

std::vector<pthread_t> threads;

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

    SERVER_SOCKET = socket_fd;
}

int setup()
{
    int ret = setup_network();

    if(ret)
        return ret;

    // other stuff

    return ret;
}

void* thread_routine(void* arg)
{
    int client_fd = (int)(long)arg;

    // step 1 is to send the current text to them as data
    

    // now in the loop just handle the commands as they come in
    while(1)
    {

    }

    return client_fd;
}

int server_entrypoint()
{
    setup();

    struct sockaddr_storage client_addr;
    socklen_t client_addr_size;

    while(1)
    {
        client_addr_size = sizeof(client_addr);
        int client_fd = accept(SERVER_SOCKET, (struct sockaddr*)&client_addr, &client_addr_size);

        if(client_fd == -1)
        {
            perror("accept");
            continue;
        }

        threads.push_back();
        pthread_create(&threads.back(), NULL, thread_routine, (void*)(long)client_fd);
    }

    for(pthread_t & thread : threads)
    {
        int* sock = new int;

        pthread_join(thread, &sock);

        close(*sock);
        delete sock;
    }

    return 0;
}

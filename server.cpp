#include "server.h"

extern std::string SERVER_PORT;

namespace server
{

    int SERVER_SOCKET;

    std::vector<pthread_t> threads;

    TextContainer<BlockingVector> text;

    void sigterm_handler(int sig)
    {
        fprintf(stderr, "Got SIGTERM\n");
        fclose(stderr);
        fclose(stdout);

        exit(1);
    }

    // credit to https://beej.us/guide/bgnet/output/html/multipage/clientserver.html
    int setup_network()
    {
        struct addrinfo hints, *server_info, *trav;

        memset(&hints, 0, sizeof(hints));

        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        int ret = getaddrinfo(nullptr, SERVER_PORT.c_str(), &hints, &server_info);

        if(ret)
        {
            fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(ret));
            return 1;
        }

        int socket_fd;
        int yes = 1;

        for(trav = server_info; trav != nullptr; trav = trav->ai_next) 
        {
            if ((socket_fd = socket(trav->ai_family, trav->ai_socktype, trav->ai_protocol)) == -1) 
            {
                perror("server: socket");
                continue;
            }

            if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1 //|| 
                    //setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY, (void*)&yes, sizeof(int)) == -1
                    || setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int)) == -1 
                )
            {
                perror("setsockopt");
                return 2;
            }

            if (bind(socket_fd, trav->ai_addr, trav->ai_addrlen) == -1) 
            {
                perror("server: bind");
                close(socket_fd);
                continue;
            }

            // if we are here then our socket is good
            break;
        }

        freeaddrinfo(server_info);

        if(trav == nullptr)
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

        std::cout << "Finished setting up server's network" << std::endl;

        return 0;
    }

    int setup()
    {
        int ret = setup_network();

        if(ret)
            return ret;

        // other stuff

        signal(SIGTERM, sigterm_handler);
        //signal(SIGINT, SIG_IGN);
        signal(SIGINT, sigterm_handler);


        std::cout << "Finished setting up server" << std::endl;

        return ret;
    }

    void* thread_routine(void* arg)
    {
        int client_fd = (int)(long)arg;

        std::cout << "Thread starting up" << std::endl;

        // step 1 is to send the current text to them as data

        std::pair<char*, size_t> prev;
        // now in the loop just handle the commands as they come in
        while(1)
        {
            std::pair<char*, size_t> msg = get_message(client_fd, true);

            // returns non-zero in second arg if it was error, else just no data
            // annoying here but used for the client code to not block
            if(msg.first == nullptr && msg.second)
            {
                fprintf(stderr, "[!!] Server got bad message!\n");
                continue;
            }
            else if (msg.first == nullptr)
            {
                // just a normal non-block
                continue;
            }

            PacketType type = get_bytes_as<PacketType>(msg.first, 0);

            //fprintf(stdout, "Got message of type %s\n", PacketTypeNames[(short)type].c_str());

            switch(type)
            {
                case PacketType::Move:
                {
                    //fprintf(stdout, "Got move!\n");
                    size_t y, x;
                    y = get_bytes_as<size_t>(msg.first, sizeof(short));
                    x = get_bytes_as<size_t>(msg.first, sizeof(short) + sizeof(size_t));

                    text.move(y, x);

                    break;
                }

                case PacketType::Insert:
                {
                    //fprintf(stdout, "Got insert!\n");

                    size_t y, x;
                    int c;

                    y = get_bytes_as<size_t>(msg.first, sizeof(short));
                    x = get_bytes_as<size_t>(msg.first, sizeof(short) + sizeof(size_t));
                    c = get_bytes_as<int>(msg.first, sizeof(short) + sizeof(size_t) * 2);

                    text.insert(y, x, c);

                    break;
                }

                case PacketType::Remove:
                {
                    //fprintf(stdout, "Got remove!\n");

                    size_t y, x;
                    y = get_bytes_as<size_t>(msg.first, sizeof(short));
                    x = get_bytes_as<size_t>(msg.first, sizeof(short) + sizeof(size_t));

                    text.remove(y, x);

                    break;
                }

                case PacketType::WriteToDisk:
                {
                    // rest of bytes are a filename
                    std::string filename(msg.first + sizeof(short), msg.second - sizeof(short));

                    printf("Writing text to disk, into %s\n", filename.c_str());

                    text.writeToFile(filename);

                    send_write_confirm(client_fd, filename);

                    break;
                }

                case PacketType::ReadFromDisk:
                {
                    // rest of bytes are a filename

                    std::string filename(msg.first + sizeof(short), msg.second - sizeof(short));

                    size_t lines = text.readFromFile(filename);

                    send_read_confirm(client_fd, lines, filename);

                    break;
                }

                case PacketType::GetFull:
                {
                    // send a full to them
                    send_full_content(client_fd, text);

                    break;
                }

                default:

                    fprintf(stderr, "[!] Server got unhandled message type '%d'\n", type);
                    break;
            }

            prev = msg;
            free_message(msg.first);
            //usleep(1000 * 10);   // 1ms * 10
        }

        std::cout << "Thread shutting down" << std::endl;

        return (void*)(long)client_fd;
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

            int yes = 1, no = 0;
            if (setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, (void*)&no, sizeof(int)) == -1)
            {
                perror("setsockopt");
                return 5;
            }

            threads.push_back(pthread_t());
            pthread_create(&threads.back(), nullptr, thread_routine, (void*)(long)client_fd);
        }

        std::cout << "Joining threads" << std::endl;

        for(pthread_t & thread : threads)
        {
            int* sock = new int;

            pthread_join(thread, (void**)&sock);

            close(*sock);
            delete sock;
        }

        std::cout << "Serving done" << std::endl;

        return 0;
    }
}

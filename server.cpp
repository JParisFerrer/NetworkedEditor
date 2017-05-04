#include "server.h"

extern std::string SERVER_PORT;
extern volatile bool SHUTDOWN_NETWORK;

namespace server
{
    int SERVER_SOCKET;

    std::vector<pthread_t> threads;

    TextContainer<BlockingVector> text;

    std::vector<Client> clients;
    std::mutex clock;
    int Client::nextid = 0;

    void sigterm_handler(int sig)
    {
        log("Got SIGTERM");
        fclose(stderr);
        fclose(stdout);

        exit(1);
    }

    // credit to https://beej.us/guide/bgnet/output/html/multipage/clientserver.html
    int setup_network()
    {
        SHUTDOWN_NETWORK = false;


        struct addrinfo hints, *server_info, *trav;

        memset(&hints, 0, sizeof(hints));

        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        int ret = getaddrinfo(nullptr, SERVER_PORT.c_str(), &hints, &server_info);

        if(ret)
        {
            log("getaddrinfo failed: %s", gai_strerror(ret));
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

            if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1 || 
                    setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY, (void*)&yes, sizeof(int)) == -1
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
            log("Server failed to bind");
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

        //signal(SIGTERM, sigterm_handler);
        //signal(SIGINT, SIG_IGN);
        signal(SIGINT, sigterm_handler);


        std::cout << "Finished setting up server" << std::endl;

        return ret;
    }

    std::vector<int> get_socket_list()
    {
        std::vector<int> ret;

        for(Client& c : clients)
            if(c.alive)
                ret.push_back(c.socket);

        return ret;
    }

    void forward_message(std::pair<char*, size_t> msg, int ignore)
    {
        std::lock_guard<std::mutex> m(clock);

        for (Client& c : clients)
        {
            if(c.id != ignore && c.alive)
            {
                //log("Forwarded message from %d to %d (sock: %d)", ignore, c.id, c.socket);
                send_message(c.socket, msg.first, msg.second);
            }
        }

    }

    void broadcast_message(char* buf, size_t len)
    {
        std::lock_guard<std::mutex> m(clock);

        log("Broadcasting to: ");

        for (Client& c : clients)
        {
            log("\t id: %d, alive: %s, sock: %d", c.id, (c.alive ? "yes": " no"), c.socket);
            if(c.alive)
                send_message(c.socket, buf, len);
        }
    }

    void* thread_routine(void* arg)
    {
        int client_idx = (int)(long)arg;
        int client_fd = clients[client_idx].socket;

        //std::cout << "Thread starting up" << std::endl;
        printf("Thread starting up for client %d on socket %d", clients[client_idx].id, client_fd);

        // step 1 is to send the current text to them as data

        std::pair<char*, size_t> prev;
        // now in the loop just handle the commands as they come in
        while(1)
        {
            //log("getting message");
            std::pair<char*, size_t> msg = get_message(client_fd, true);

            // returns non-zero in second arg if it was error, else just no data
            // annoying here but used for the client code to not block
            if(msg.first == nullptr && msg.second > 0)
            {
                log("[!!] Server had trouble reading message!");
                break;
            }
            else if (msg.first == nullptr && msg.second == -1)
            {
                // they disconnected, probably

                break;
            }
            else if (msg.first == nullptr)
            {
                break;
                //continue;
            }

            PacketType type = get_bytes_as<PacketType>(msg.first, 0);

            //fprintf(stdout, "Got message of type %s", PacketTypeNames[(short)type].c_str());

            switch(type)
            {
                case PacketType::Move:
                {
                    //fprintf(stdout, "Got move!");
                    size_t y, x;
                    y = get_bytes_as<size_t>(msg.first, sizeof(short));
                    x = get_bytes_as<size_t>(msg.first, sizeof(short) + sizeof(size_t));

                    //text.move(y, x);

                    break;
                }

                case PacketType::Insert:
                {
                    //fprintf(stdout, "Got insert!");

                    size_t y, x;
                    int c;

                    y = get_bytes_as<size_t>(msg.first, sizeof(short));
                    x = get_bytes_as<size_t>(msg.first, sizeof(short) + sizeof(size_t));
                    c = get_bytes_as<int>(msg.first, sizeof(short) + sizeof(size_t) * 2);

                    text.insert(y, x, c);

                    forward_message(msg, clients[client_idx].id);

                    break;
                }

                case PacketType::Remove:
                {
                    //fprintf(stdout, "Got remove!");

                    size_t y, x;
                    y = get_bytes_as<size_t>(msg.first, sizeof(short));
                    x = get_bytes_as<size_t>(msg.first, sizeof(short) + sizeof(size_t));

                    text.remove(y, x);

                    forward_message(msg, clients[client_idx].id);

                    break;
                }

                case PacketType::WriteToDisk:
                {
                    // rest of bytes are a filename
                    std::string filename(msg.first + sizeof(short));

                    printf("Writing text to disk, into %s", filename.c_str());

                    text.writeToFile(filename);

                    //send_write_confirm(client_fd, filename);
                    broadcast_write_confirm(get_socket_list(), filename);

                    break;
                }

                case PacketType::ReadFromDisk:
                {
                    // rest of bytes are a filename

                    std::string filename(msg.first + sizeof(short));

                    size_t lines = text.readFromFile(filename);

                    broadcast_read_confirm(get_socket_list(), lines, filename);
                    broadcast_full_content(get_socket_list(), text, true);

                    break;
                }

                case PacketType::GetFull:
                {
                    // send a full to them
                    //log("got full request");
                    send_full_content(client_fd, text, false);

                    break;
                }

                case PacketType::GetClientCount:
                {
                    log("sending client count of %lu", get_socket_list().size());

                    send_client_count(client_fd, get_socket_list().size());

                    break;
                }

                default:

                    log("[!] Server got unhandled message type '%s' (%d)", ((short)type < PacketTypeNum ? PacketTypeNames[(short)type].c_str() : ""), (short)type);
                    break;
            }

            prev = msg;
            free_message(msg.first);
            //usleep(1000 * 10);   // 1ms * 10
        }

        //std::cout << "Thread shutting down" << std::endl;
        printf("Thread handling client %d (sock %d) shutting down", clients[client_idx].id, client_fd);

        broadcast_disconnect(get_socket_list());

        clients[client_idx].alive = false;

        // try to save the results
        log("Server trying to save autosave file on exit: 'autosave.txt'");
        text.writeToFile("autosave.txt");

        return (void*)(long)client_fd;
    }

    int server_entrypoint()
    {
        int ret = setup();

        if(ret)
            return ret;

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
            if (setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, (void*)&yes, sizeof(int)) == -1)
            {
                perror("setsockopt");
                return 5;
            }

            broadcast_new_client(get_socket_list());

            clock.lock();
            clients.push_back(Client(client_fd));
            clock.unlock();

            threads.push_back(pthread_t());
            pthread_create(&threads.back(), nullptr, thread_routine, (void*)(long)(clients.size()-1));
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

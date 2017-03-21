int setup_network()
{

}

int setup()
{
    int ret = setup_network();

    if(ret)
        return ret;

    // other stuff

    return ret;
}

int server_entrypoint(int port)
{
    setup();

    return 0;
}

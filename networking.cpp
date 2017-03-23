#include "networking.h"


bool send_bytes(int sock, char* buf, size_t num_bytes)
{
    size_t left = num_bytes;
    size_t total_sent = 0;
    int mtu = MTU;

    while(left > 0)
    {
        int flags = (left > mtu ? MSG_MORE : 0);
        ssize_t sent = send(sock, buf + total_sent, std::min(left, (size_t)mtu), flags);

        if(sent == -1)
        {
            //errors

            if(errno == EMSGSIZE)
            {
                mtu = (int)(mtu*0.9);
            }
            else 
            {
                return false;
            }
        }
        else
        {
            total_sent += sent;
            left -= sent;
        }
    }

    return true;
}

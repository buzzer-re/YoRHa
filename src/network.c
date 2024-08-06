#include "../include/network.h"


//
// Create a socket and bind to given port, returns the sock
//
int listen_port(int port, struct thread* td, int nonblock)
{
    int sock = ksocket(AF_INET, SOCK_STREAM, 0, td);

    if (sock < 0)
    {
        kprintf("Unable to create socket!\n");
        return -1;
    } 

    if (nonblock)
    {

    }


    struct sockaddr_in sockaddr;
    socklen_t socklen = sizeof(sockaddr);
    sockaddr.sin_len = socklen;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = __builtin_bswap16(port);
    sockaddr.sin_addr.s_addr  = __builtin_bswap32(INADDR_ANY);
    
    if (kbind(sock, (struct sockaddr*) &sockaddr, socklen, td) < 0)
    {
        kprintf("Unable to bind socket %d on port %d\n", sock, port);
        kclose(sock, td);
        return -1;
    }

    if (klisten(sock, 4,td) < 0)
    {
        kprintf("Unable to listen socket %d on port %d\n",sock, port);
        kclose(sock, td);
        return -1;
    }

    return sock;
}
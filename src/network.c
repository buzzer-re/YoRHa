#include "../include/network.h"


//
// Create a socket and bind to given port, returns the sock
//
int listen_port(int port, struct thread* td, int reuse)
{
    int sock = ksocket(AF_INET, SOCK_STREAM, 0, td);

    if (sock < 0)
    {
        kprintf("Unable to create socket!\n");
        return -1;
    } 
    
    struct sockaddr_in sockaddr;
    socklen_t socklen = sizeof(sockaddr);
    sockaddr.sin_len = socklen;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = __builtin_bswap16(port);
    sockaddr.sin_addr.s_addr  = __builtin_bswap32(INADDR_ANY);
    
    if (reuse)
    {
        ksetsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int), td);
    }
    
    if (kbind(sock, (struct sockaddr*) &sockaddr, socklen, td) < 0)
    {
        kprintf("Unable to bind socket %d on port %d\n", sock, port);
        kshutdown(sock, SHUT_RDWR, td);
        kclose(sock, td);
        return -1;
    }

    if (klisten(sock, 4,td) < 0)
    {
        kprintf("Unable to listen socket %d on port %d\n",sock, port);
        kshutdown(sock, SHUT_RDWR, td);
        kclose(sock, td);
        return -1;
    }

    return sock;
}

// size_t get_read_size(int sock, int timeout)
// {
//     int old_flags = kfcntl(sock, F_GETFL, NULL, curthread);
//     if (!old_flags)
//     {
//         kprintf("get_read_size(): Invalid socket!\n");
//         return 0;
//     }
//     //
//     // Set to non-blocking
//     //

//     fd_set readfds;
//     int status;
//     struct timeval tv;
    
//     FD_ZERO(&readfds);
//     FD_SET(sock, &readfds);

//     tv.tv_sec = timeout;
//     tv.tv_usec = 0;

//     if (! (old_flags & O_NONBLOCK) )
//         kfcntl(sock, F_SETFL, old_flags | O_NONBLOCK, curthread);

//     int status = kselect(sock + 1, &readfds, NULL, NULL, &tv, curthread);

//     if (status <= 0)
//     {
//         kprintf("get_read_size(): Invalid socket! on select()\n");
//         return 0;
//     }


// }
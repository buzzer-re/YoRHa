#pragma once

#include "kernel.h"
#include "common.h"
#include <sys/socket.h>
#include <sys/sysproto.h>
#include <sys/proc.h>


int ksocket(int domain, int type, int protocol, struct thread* td);
int kclose(int socket, struct thread* td);
int kbind(int socket, struct sockaddr* addr, size_t addrlen, struct thread* td);
int klisten(int socket, int backlog, struct thread* td);
int kread(int fd, void *buf, size_t count, struct thread* td);
int kaccept(int sockfd, struct sockaddr *addr, socklen_t *addrlen, struct thread* td);
int krecvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, size_t *addrlen, struct thread* td);
int ksendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen, struct thread* td);
int kfcntl(int fd, int cmd, long arg, struct thread* td);
int kselect(int  nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout, struct thread* td);
int kshutdown(int sockfd, int how, struct thread* td);
int ksetsockopt(int s, int level, int optname, const void *optval, socklen_t optlen, struct thread* td);
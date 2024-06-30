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
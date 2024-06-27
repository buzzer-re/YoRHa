#pragma once

#include "kernel.h"
#include "common.h"
#include <sys/socket.h>
#include <sys/sysproto.h>
#include <sys/proc.h>


int ksocket(int domain, int type, int protocol, struct thread* td);
int kclose(int socket, struct thread* td);
int kbind(int socket, struct sockaddr addr, struct thread* td);
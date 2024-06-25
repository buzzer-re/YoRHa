#pragma once

#include "kernel.h"
#include "common.h"
#include <sys/socket.h>
#include <sys/sysproto.h>
#include <sys/proc.h>


int ksocket(int domain, int type, int protocol, struct thread* td);
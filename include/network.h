#pragma once

#include <sys/fcntl.h>
#include "common.h"
#include "kernel.h"
#include "syscall_wrapper.h"
#define _KERNEL
#include <sys/pcpu.h>


#define TIMEOUT_NOW 1

int listen_port(int port, struct thread* td, int reuse);
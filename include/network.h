#pragma once

#include <sys/fcntl.h>
#include "common.h"
#include "kernel.h"
#include "syscall_wrapper.h"
#define _KERNEL
#include <sys/pcpu.h>


#define TIMEOUT_NOW 1

int listen_port(int port, struct thread* td, int nonblock);
// size_t get_read_size(int sock, int timeout);
// int read_non_block_connection(uint8_t* buff, size_t buff_size, int conn, struct thread* td)
// int write_non_block_connection(uint8_t* buff, size_t buff_size, int conn, struct thread* td)
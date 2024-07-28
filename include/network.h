#pragma once

#include "common.h"
#include "kernel.h"
#include "syscall_wrapper.h"
#define _KERNEL
#include <sys/pcpu.h>

int listen_port(int port);
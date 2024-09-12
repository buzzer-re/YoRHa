#pragma once

#include "../cpu.h"

struct dbg_command_t;
struct trap_frame_t;
struct dbg_response_header;

#define PAUSE_KERNEL_CODE_DUMP_SIZE 0x100

typedef struct __attribute__((__packed__)) __pause_kernel_data
{
    dbg_response_header header;
    trap_frame_t trap_frame;
} pause_kernel_response_data_t;

int pause_kernel_executor(dbg_command_t*, int);
int pause_kernel_trap_handler(dbg_command_t*, int, trap_frame_t*);
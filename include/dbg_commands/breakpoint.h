#pragma once

#include <sys/stdint.h>

struct dbg_command;
struct trap_frame_t;

int place_breakpoint_executor(dbg_command_t*, int);
int place_breakpoint_trap_handler(dbg_command_t*, int, trap_frame_t*);

typedef struct __attribute__((__packed__)) __breakpoint_request
{
    uint64_t* target_address;
} breakpoint_request_t;

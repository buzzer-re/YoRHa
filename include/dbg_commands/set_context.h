#pragma once

#include <sys/stdint.h>
#include "../breakpoint_manager.h"
#include "../syscall_wrapper.h"
#include "../cpu.h"

struct dbg_response_t;
struct dbg_command_t;

//
// Create new breakpoints
//
int set_thread_context_executor(dbg_command_t*, int);
int set_thread_context_trap_handler(dbg_command_t*, int, trap_frame_t*);

typedef struct __attribute__((__packed__)) __thread_context
{
    trap_frame_t ctx;
} thread_context_request_t;

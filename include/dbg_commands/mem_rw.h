#pragma once

#include <stdint.h>

struct dbg_response;
struct dbg_command;
struct trap_frame_t;
typedef dbg_response dbg_mem_read_response_t;


typedef struct __dbg_mem_read_request
{
    uint64_t*   target_addr;
    size_t      read_size;
} dbg_mem_read_request_t; 


int memory_read_executor(dbg_command*, int);
int memory_read_trap_handler(dbg_command*, int, trap_frame_t*);

// TODO: Mem write
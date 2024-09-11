#pragma once

#include <stdint.h>

struct dbg_response;
struct dbg_command_t;
struct trap_frame_t;
typedef dbg_response dbg_mem_read_response_t;


typedef struct __attribute__((__packed__)) __dbg_mem_read_request
{
    uint8_t*   target_addr;
    size_t      read_size;
} dbg_mem_read_request_t; 

typedef struct __attribute__((__packed__)) __dbg_mem_write_request
{
    uint8_t*   target_addr;
    size_t      write_size;
} dbg_mem_write_request_t; 



int memory_read_executor(dbg_command_t*, int);
int memory_read_trap_handler(dbg_command_t*, int, trap_frame_t*);

int memory_write_executor(dbg_command_t*, int);
int memory_write_trap_handler(dbg_command_t*, int, trap_frame_t*);

// TODO: Mem write
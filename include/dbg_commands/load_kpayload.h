#pragma once

struct dbg_command_t;
struct trap_frame_t;

int place_breakpoint_executor(dbg_command_t*, int);
int place_breakpoint_trap_handler(dbg_command_t*, int, trap_frame_t*);


typedef struct __attribute__((__packed__)) __kpayload_loader_request
{
    uint8_t stop_at_entry;
    uint64_t payload_size;
} kpayload_loader_request_t;


int kpayload_loader_executor(dbg_command_t*, int);
// not yet
// int kpayload_loader_trap_handler(dbg_command*, int, trap_frame_t*);

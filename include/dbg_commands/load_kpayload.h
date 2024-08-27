#pragma once

struct dbg_command;
struct trap_frame_t;

int place_breakpoint_executor(dbg_command*, int);
int place_breakpoint_trap_handler(dbg_command*, int, trap_frame_t*);

typedef struct __kpayload_loader_request
{
    uint8_t payload_begin;
} kpayload_loader_request_t;


int kpayload_loader_executor(dbg_command*, int);
// not yet
// int kpayload_loader_trap_handler(dbg_command*, int, trap_frame_t*);

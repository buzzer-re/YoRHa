#pragma once

struct dbg_command;
struct trap_frame_t;
struct dbg_response_header;

#define PAUSE_KERNEL_CODE_DUMP_SIZE 0x100
typedef struct __pause_kernel_data
{
    dbg_response_header header;
    trap_frame_t trap_frame;
    uint8_t code[PAUSE_KERNEL_CODE_DUMP_SIZE]; // a simple 64 bytes dump of the current execution code
} pause_kernel_response_data_t;

int pause_kernel_executor(dbg_command*, int);
int pause_kernel_trap_handler(dbg_command*, int, trap_frame_t*);
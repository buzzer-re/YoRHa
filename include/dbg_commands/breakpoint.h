#pragma once

#include <sys/stdint.h>
#include "../breakpoint_manager.h"

struct dbg_command_t;
struct trap_frame_t;

//
// Create new breakpoints
//
int place_breakpoint_executor(dbg_command_t*, int);
int place_breakpoint_trap_handler(dbg_command_t*, int, trap_frame_t*);

typedef struct __attribute__((__packed__)) __breakpoint_request
{
    uint64_t* target_address;
} breakpoint_request_t;

//
// List breakpoints
//
typedef struct __attribute__((__packed__)) __breakpoint_list_response
{
    dbg_response_header_t header;
    size_t num_breakpoints;
    breakpoint_entry_t breakpoint_entry;
} breakpoint_list_response_t;


int list_breakpoint_executor(dbg_command_t*, int);
int list_breakpoint_trap_handler(dbg_command_t*, int, trap_frame_t*);

//
// Remove breakpoint
//
typedef struct __attribute__((__packed__)) __breakpoint_remove_request
{
    uint64_t* target_addr;
} breakpoint_remove_request_t;

int remove_breakpoint_executor(dbg_command_t*, int);
int remove_breakpoint_trap_handler(dbg_command_t*, int, trap_frame_t*);
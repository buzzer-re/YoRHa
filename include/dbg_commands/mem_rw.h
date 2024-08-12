#pragma once


struct dbg_response;
struct dbg_command;
struct trap_frame_t;
typedef dbg_response dbg_mem_read_response_t;


int memory_read_executor(dbg_command*, int);
int memory_read_trap_handler(dbg_command*, int, trap_frame_t*);

// TODO: Mem write
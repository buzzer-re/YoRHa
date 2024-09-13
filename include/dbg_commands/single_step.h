#pragma once

#include <sys/stdint.h>
#include "../syscall_wrapper.h"

struct dbg_command_t;
struct trap_frame_t;

int single_step_executor(dbg_command_t*, int);
int single_step_trap_handler(dbg_command_t*, int, trap_frame_t*);

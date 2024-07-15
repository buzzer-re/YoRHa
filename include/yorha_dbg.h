#pragma once

#include <stdint.h>
#include <sys/types.h>
#include <sys/proc.h>
#define _KERNEL
#include <sys/pcpu.h>
#include "kernel.h"

#include "common.h"
#include "syscall_wrapper.h"
#include "yorha_dbg_commands.h"

int yorha_dbg_breakpoint_handler(trap_frame_t* ctx);
int yorha_trap_command_handler(trap_frame_t* ctx);
int yorha_dbg_run_debug_server_loop(int port);
int yorha_dbg_handle_command(dbg_command* command, int conn);
int yorha_trap_dbg_get_new_commands(uint8_t* buff, size_t buff_size);

//
// Command handlers
//



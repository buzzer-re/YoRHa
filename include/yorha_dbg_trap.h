#pragma once
#define LOCK_DEBUG 1

#include <stdint.h>
#include <sys/types.h>
#include <sys/proc.h>
#include <sys/fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#define _KERNEL
#include <sys/pcpu.h>

#include "cpu.h"
#include "kernel.h"
#include "common.h"
#include "syscall_wrapper.h"
#include "yorha_dbg_commands.h"
#include "network.h"
#include "breakpoint_manager.h"

int yorha_dbg_main_trap_handler(trap_frame_t* ctx, dbg_command_t* cmd);
int yorha_trap_command_handler(trap_frame_t* ctx);
yorha_trap_dbg_get_new_commands(dbg_command_t* command, int conn, struct thread* td);


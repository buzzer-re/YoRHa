#pragma once
#include <stdint.h>
#include <sys/types.h>
#include <sys/proc.h>
#define _KERNEL
#include <sys/pcpu.h>
#include "kernel.h"
#include "common.h"
#include "syscall_wrapper.h"

enum command_type
{
    CONTINUE = 0
};

typedef struct __trap_frame
{
    uint64_t rax;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rbp;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t error_code;
    uint64_t rip;
    uint64_t cs;
    uint64_t eflags;
    uint64_t rsp;
    uint64_t ss;
} trap_frame_ctx;

typedef struct __dbg_command_header
{
    enum command_type command_type;
    uint64_t argument_size;
} dbg_command_header;

typedef struct __dbg_command
{
    dbg_command_header header;
    uint8_t data[];
} dbg_command;



void yorha_dbg_breakpoint_handler(trap_frame_ctx* registers);
int yorha_dbg_init_debug_server(int port);

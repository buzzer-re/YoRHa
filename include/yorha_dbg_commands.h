#pragma once
#include "common.h"

enum command_type // TODO: Remove that one
{
    PAUSE_KERNEL = 0, // issue a int3 on the running kernel
    STOP_DBG, // close the debugger
};

enum DbgStatus
{
    IDLE = 0,
    RUNNING,
    STOPPED,
    ERROR
};

typedef struct __registers
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
} registers_t;

typedef struct __trap_frame
{
    registers_t regs;
} trap_frame_ctx;

typedef struct __dbg_command_header
{
    enum command_type command_type;
    uint64_t argument_size;
} dbg_command_header;

typedef struct __dbg_response_header
{
    enum command_type command_type;
    uint64_t response_size;
} dbg_response_header;

typedef struct __dbg_command
{
    dbg_command_header header;
    uint8_t data[];
} dbg_command;

typedef struct __dbg_response
{
    dbg_response_header header;
    uint8_t data[];
} dbg_response;

typedef int(*command_executor)(dbg_command*, int);
typedef int(*command_trap_handler)(dbg_command*, int, trap_frame_ctx*);

int pause_kernel_executor(dbg_command*, int);
int pause_kernel_trap_handler(dbg_command*, int, trap_frame_ctx*);
int stop_debugger_executor(dbg_command*, int);


typedef struct __pause_kernel_data
{
    dbg_response_header header;
    registers_t regs;
    uint8_t code[0x10]; // a simple 64 bytes dump of the current execution code
} pause_kernel_response_data_t;


static void* command_executor_handlers[] = 
{
    pause_kernel_executor,
    stop_debugger_executor
};

static void* command_trap_handlers[] = 
{
    pause_kernel_trap_handler, // 0
    NULL, // This will never be called
};

enum dbg_commands_code  {
    DBG_PAUSE = 0,
    DBG_PLACE_BREAKPOINT = 1,
};

#define __max_dbg_commands sizeof(command_executor_handlers)/sizeof(command_executor_handlers[0])
#define __max_dbg_trap_handlers sizeof(command_trap_handlers)/sizeof(command_trap_handlers[0])


//
// asm helpers
//
extern void __debugbreak();


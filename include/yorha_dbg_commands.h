#pragma once
#include "common.h"


enum dbg_commands_code  {
    DBG_PAUSE = 0,
    DBG_STOP,
    DBG_PLACE_BREAKPOINT,
    DBG_CONTINUE,
};

enum DbgStatus
{
    IDLE = 0,
    RUNNING,
    STOPPED,
    ERROR
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
} trap_frame_t;


typedef struct __dbg_command_header
{
    enum dbg_commands_code command_type;
    uint64_t argument_size;
} dbg_command_header;


typedef struct __dbg_response_header
{
    enum dbg_commands_code command_type;
    int command_status;
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
typedef int(*command_trap_handler)(dbg_command*, int, trap_frame_t*);

int pause_kernel_executor(dbg_command*, int);
int pause_kernel_trap_handler(dbg_command*, int, trap_frame_t*);
int place_breakpoint_executor(dbg_command*, int);
int place_breakpoint_trap_handler(dbg_command*, int, trap_frame_t*);

int stop_debugger_executor(dbg_command*, int);

/////////////// PAUSE KERNEL RESPONSE //////////////
#define PAUSE_KERNEL_CODE_DUMP_SIZE 0x100
typedef struct __pause_kernel_data
{
    dbg_response_header header;
    trap_frame_t trap_frame;
    uint8_t code[PAUSE_KERNEL_CODE_DUMP_SIZE]; // a simple 64 bytes dump of the current execution code
} pause_kernel_response_data_t;
/////////////// PAUSE KERNEL RESPONSE END ///////////


////////////// PLACE BREAKPOINT REQUEST /////////////

typedef struct __breakpoint_request
{
    uint64_t* target_address;
} breakpoint_request_t;


static void* command_executor_handlers[] = 
{
    pause_kernel_executor,
    stop_debugger_executor,
    place_breakpoint_executor,
};

static void* command_trap_handlers[] = 
{
    pause_kernel_trap_handler, // 0
    NULL, // This will never be called,
    place_breakpoint_trap_handler
};


#define __max_dbg_commands sizeof(command_executor_handlers)/sizeof(command_executor_handlers[0])
#define __max_dbg_trap_handlers sizeof(command_trap_handlers)/sizeof(command_trap_handlers[0])


//
// asm helpers
//
extern void __debugbreak();


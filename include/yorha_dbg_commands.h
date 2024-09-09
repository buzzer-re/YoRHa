#pragma once
#include "common.h"


enum dbg_commands_code  {
    DBG_PAUSE = 0,
    DBG_STOP,
    DBG_PLACE_BREAKPOINT,
    DBG_CONTINUE,
    DBG_CONTEXT,
    DBG_MEM_READ,
    DBG_KPAYLOAD_LOADER,
    DBG_LIST_BREAKPOINT,
    DBG_REMOVE_BREAKPOINT
};

enum DbgStatus
{
    IDLE = 0,
    RUNNING,
    STOPPED,
    ERROR
};


typedef struct __attribute__((__packed__)) __trap_frame
{
    uint64_t rax;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rbx;
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
   // uint64_t error_code;
    uint64_t rip;
    uint64_t cs;
    uint64_t eflags;
    uint64_t rsp;
    uint64_t ss;
} trap_frame_t;


typedef struct __attribute__((__packed__)) __dbg_command_header
{
    enum dbg_commands_code command_type;
    uint64_t argument_size;
} dbg_command_header;

typedef struct __attribute__((__packed__)) __dbg_response_header
{
    enum dbg_commands_code command_type;
    int command_status;
    uint64_t response_size;
} dbg_response_header, dbg_response_header_t;


typedef struct __attribute__((__packed__)) __dbg_command
{
    dbg_command_header header;
} dbg_command_t;


typedef struct __attribute__((__packed__)) __dbg_response
{
    dbg_response_header header;
    uint8_t data[];
} dbg_response;



typedef int(*command_executor)(dbg_command_t*, int);
typedef int(*command_trap_handler)(dbg_command_t*, int, trap_frame_t*);


#include "dbg_commands/pause.h"
#include "dbg_commands/breakpoint.h"
#include "dbg_commands/stop.h"
#include "dbg_commands/mem_rw.h"
#include "dbg_commands/load_kpayload.h"


static void* command_executor_handlers[] = 
{
    pause_kernel_executor,
    stop_debugger_executor,
    place_breakpoint_executor,
    NULL, // continue
    memory_read_executor,
    kpayload_loader_executor,
    list_breakpoint_executor,
    remove_breakpoint_executor
};

static void* command_trap_handlers[] = 
{
    pause_kernel_trap_handler, // 0
    NULL, // This will never be called,
    place_breakpoint_trap_handler,
    NULL, // continue
    memory_read_trap_handler,
    NULL,
    list_breakpoint_trap_handler,
    remove_breakpoint_trap_handler
};


#define __max_dbg_commands sizeof(command_executor_handlers)/sizeof(command_executor_handlers[0])
#define __max_dbg_trap_handlers sizeof(command_trap_handlers)/sizeof(command_trap_handlers[0])


//
// asm helpers
//
extern void __debugbreak();


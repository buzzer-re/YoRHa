#pragma once
#include "common.h"
#include "cpu.h"

enum dbg_commands_code  {
    DBG_PAUSE = 0,
    DBG_STOP,
    DBG_PLACE_BREAKPOINT,
    DBG_CONTINUE,
    DBG_CONTEXT,
    DBG_MEM_READ,
    DBG_KPAYLOAD_LOADER,
    DBG_LIST_BREAKPOINT,
    DBG_REMOVE_BREAKPOINT,
    DBG_MEM_WRITE,
    DBG_SET_THREAD_CONTEXT
};

enum DbgStatus
{
    IDLE = 0,
    RUNNING,
    STOPPED,
    ERROR
};

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
#include "dbg_commands/set_context.h"


static void* command_executor_handlers[] = 
{
    pause_kernel_executor,
    stop_debugger_executor,
    place_breakpoint_executor,
    NULL, // continue
    memory_read_executor,
    kpayload_loader_executor,
    list_breakpoint_executor,
    remove_breakpoint_executor,
    memory_write_executor,
    NULL
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
    remove_breakpoint_trap_handler,
    memory_write_trap_handler,
    set_thread_context_trap_handler
};


#define __max_dbg_commands sizeof(command_executor_handlers)/sizeof(command_executor_handlers[0])
#define __max_dbg_trap_handlers sizeof(command_trap_handlers)/sizeof(command_trap_handlers[0])


//
// asm helpers
//
extern void __debugbreak();


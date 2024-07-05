#pragma once


typedef int(*command_handler)(dbg_command*, int);

int pause_kernel(dbg_command*, int);

__attribute__((unused)) static void* command_handlers[] = 
{
    pause_kernel,
};

enum dbg_commands_code  {
    DBG_PAUSE = 0,
    DBG_PLACE_BREAKPOINT = 1,
};

#define __max_dbg_commands sizeof(command_handlers)/sizeof(command_handlers[0])

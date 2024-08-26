#pragma once

#include <sys/queue.h>
#include "common.h"
#include "kernel.h"
#include "cpu.h"



typedef struct __breakpoint_entry
{
    uint64_t* address;
    uint8_t old_opcode;
    uint8_t enabled;
    SLIST_ENTRY(__breakpoint_entry) entries;
} breakpoint_entry_t;


int add_breakpoint(uint64_t* addr);




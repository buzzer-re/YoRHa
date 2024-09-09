#pragma once

#include <sys/queue.h>
#include "common.h"
#include "kernel.h"
#include "cpu.h"



typedef struct __attribute__((__packed__)) __breakpoint_entry
{
    uint64_t* address;
    uint8_t old_opcode;
    uint8_t enabled;
    SLIST_ENTRY(__breakpoint_entry) entries;
} breakpoint_entry_t;


int add_breakpoint(uint64_t* addr);
int remove_breakpoint(uint64_t* addr);
breakpoint_entry_t* get_breakpoint_entry(uint64_t* addr);
breakpoint_entry_t* get_breakpoint_addresses(size_t* len);



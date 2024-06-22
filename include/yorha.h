#pragma once

#include "../include/intrin.h"
#include "../include/kernel.h"
#include "../include/yorha_dbg.h"

enum 
{
    YORHA_SUCCESS = 0,
    YORHA_FAILURE,
};

extern void (*int_breakpoint_handler)();

#define LOG(msg, ...) kprintf("YorhaDBG: "msg"\n", __VA_ARGS__)

int yorha_dbg_init();
void overwrite_idt_gate(int interruption_number, uint64_t gate_addr);

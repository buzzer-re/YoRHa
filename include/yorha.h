#pragma once

#include "intrin.h"
#include "kernel.h"
#include "yorha_dbg_ctrl.h"
#include "common.h"

extern void (*debug_int_handler)();
extern uint64_t __get_rsp();
extern uint64_t __get_rip();

#define LOG(msg, ...) kprintf("YorhaDBG: "msg"\n", __VA_ARGS__)

int yorha_dbg_start();
int init_debug_server();
uint64_t overwrite_idt_gate(int interruption_number, uint64_t gate_addr);

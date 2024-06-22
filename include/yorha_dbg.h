#pragma once
#include <stdint.h>
#include "../include/kernel.h"

typedef struct _X64Registers
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
} X64Registers;


void yorha_dbg_breakpoint_handler(X64Registers* registers);
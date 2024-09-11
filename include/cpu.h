#pragma once

#include "common.h"
#include "kernel.h"
#include "machine/apic.h"

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#define _KERNEL
#include <sys/pcpu.h>
#include <sys/cpuset.h>


#define STOP() stop_other_cpus()
#define RESTART() restart_cpus()
#define disable_thread_pf() curthread_pflags_set(TDP_NOFAULTING)
#define update_thread_flags(f) curthread_pflags_restore(f)

extern int cpu_stopped;

int stop_other_cpus();
int restart_cpus();

int curthread_pflags_set(int flags);
void curthread_pflags_restore(int save);


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



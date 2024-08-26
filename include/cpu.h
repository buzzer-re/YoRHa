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



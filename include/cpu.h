#pragma once

#include "common.h"
#include "kernel.h"
#include "machine/apic.h"

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/param.h>
#define _KERNEL
#include <sys/pcpu.h>
#include <sys/cpuset.h>

int stop_other_cpus();
int restart_cpus();
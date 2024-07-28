#include "../include/cpu.h"

cpuset_t all_cpus;
volatile cpuset_t stopped_cpus;

//
// Stop all running CPUs (besides the current one) using the IPI request wrapper "generic_stop_cpus" from freebsd
//
int stop_other_cpus()
{
    //
    // TODO: Get the all_cpus without use the global variable offset
    //
    all_cpus = (cpuset_t) *((cpuset_t*) (kernel_base + all_cpus_offset));
    stopped_cpus = (cpuset_t) *((cpuset_t*) (kernel_base + stopped_cpus_offset));
    cpuset_t other_cpus;
    other_cpus = all_cpus;
    
    //
    // Remove the current CPU
    //
    CPU_CLR(PCPU_GET(cpuid), &other_cpus);

    return kgeneric_stop_cpus(other_cpus, IPI_STOP_HARD);
}


//
// Restart CPUs previously stopped
//
int restart_cpus()
{
    stopped_cpus = (cpuset_t) *((cpuset_t*) (kernel_base + stopped_cpus_offset));
    return krestart_cpus(stopped_cpus);
}
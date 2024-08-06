#include "../include/cpu.h"

cpuset_t all_cpus;
volatile cpuset_t stopped_cpus;
int cpu_stopped = false;

//
// Stop all running CPUs (besides the current one) using the IPI request wrapper "generic_stop_cpus" from freebsd
//
int stop_other_cpus()
{
    if (!cpu_stopped)
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

        cpu_stopped = true;
        return kgeneric_stop_cpus(other_cpus, IPI_STOP_HARD);
    }

    return -1;
}


//
// Restart CPUs previously stopped
//
int restart_cpus()
{   
    if (cpu_stopped)
    {
        cpu_stopped = false;

        stopped_cpus = (cpuset_t) *((cpuset_t*) (kernel_base + stopped_cpus_offset));
        return krestart_cpus(stopped_cpus);
    }

    return -1;
}
#pragma once

#include <stdint.h>
#include "machine/idt.h"

#define X86_CR0_WP (1 << 16)
// MSRs


//
// This intrinsics code make use of the GAS syntax, which the whole FreeBSD uses, it will make our compilation less error-prone (trust me)
//

static inline __attribute__((always_inline)) uint64_t __readmsr(unsigned long __register) {
  unsigned long __edx;
  unsigned long __eax;
  __asm__("rdmsr"
          : "=d"(__edx), "=a"(__eax)
          : "c"(__register));
  return (((uint64_t)__edx) << 32) | (uint64_t)__eax;
}


static inline __attribute__((always_inline)) uint64_t __readcr0(void) {
  uint64_t cr0;
  __asm__ volatile("movq %%cr0, %0"
                   : "=r"(cr0)
                   :
                   : "memory");
  return cr0;
}

static inline __attribute__((always_inline)) void __writecr0(uint64_t cr0) {
  __asm__ volatile("movq %0, %%cr0"
                   :
                   : "r"(cr0)
                   : "memory");
}

static inline __attribute__((always_inline)) void __sidt(IDTR* idtr) {
    __asm__ volatile("sidt %0" :: "m"(*idtr));
}

#pragma once

#include <stdint.h>
#include "machine/idt.h"

#define __readcr0 readCr0
#define __writecr0 writeCr0
#define X86_CR0_WP (1 << 16)

static inline __attribute__((always_inline)) uint64_t readCr0(void) {
  uint64_t cr0;
  __asm__ volatile("movq %0, %%cr0"
                   : "=r"(cr0)
                   :
                   : "memory");
  return cr0;
}

static inline __attribute__((always_inline)) void writeCr0(uint64_t cr0) {
  __asm__ volatile("movq %%cr0, %0"
                   :
                   : "r"(cr0)
                   : "memory");
}
// MSRs
#define MSR_LSTAR 0xC0000082



//
// instructions wrappers
//
void __sidt(IDTR* idtr);

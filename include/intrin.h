#pragma once

#include <stdint.h>
#include "machine/idt.h"
#include "payload_utils.h"

#define __readcr0 readCr0
#define __writecr0 writeCr0


// MSRs
#define MSR_LSTAR 0xC0000082



//
// instructions wrappers
//
void __sidt(IDTR* idtr);
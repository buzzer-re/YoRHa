#pragma once

#include <stdint.h>
#include "machine/idt.h"

void __sidt(struct IDTR* idtr);
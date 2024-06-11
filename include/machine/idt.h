#pragma once

#include <stdint.h>

struct IDTR
{
    uint16_t  limit;      /* Size of IDT array - 1 */
    uint64_t base;       /* Pointer to IDT array  */
} __attribute__((packed));
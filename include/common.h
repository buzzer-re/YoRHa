#pragma once
#include <stddef.h>
#include <stdbool.h>

#define DBG_TRAP_PORT 8880
#define DBG_CTRL_PORT 8882

enum 
{
    YORHA_SUCCESS = 0,
    YORHA_FAILURE,
};

extern int memcpy(void *dest, const void *src, size_t n);
#define UNUSED __attribute__((unused))
#define INT3 0xCC
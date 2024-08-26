#pragma once
#include <sys/types.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define DBG_TRAP_PORT 8888
#define DBG_CTRL_PORT 8887

enum 
{
    YORHA_SUCCESS = 0,
    YORHA_FAILURE,
    YORHA_INVALID_MEM_ADDRESS
};

extern int memcpy(void *dest, const void *src, size_t n);
#define UNUSED __attribute__((unused))
#define INT3 0xCC
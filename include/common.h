#pragma once

enum 
{
    YORHA_SUCCESS = 0,
    YORHA_FAILURE,
};

extern int memcpy(void *dest, const void *src, size_t n);
#define UNUSED __attribute__((unused))
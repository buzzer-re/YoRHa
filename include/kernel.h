#pragma once

#include "intrin.h"
#include "ps4.h"

#define DISABLE_WP() \
        __asm__("cli");\
        disable_cr0_wp()\

#define ENABLE_WP()\
        __asm__("sti");\
        enable_cr0_wp()\

#define kprintf_offset 0xffffffff822b7a30

extern int (*kprintf)(const char *format, ...);


void init_kernel();
void enable_safe_patch();
void disable_safe_patch();
void disable_cr0_wp();
void enable_cr0_wp();

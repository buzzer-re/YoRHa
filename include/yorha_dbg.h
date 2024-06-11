#pragma once

#include "ps4.h"
#include "../include/intrin.h"

enum 
{
    YORHA_FAILURE = 0,
    YORHA_SUCCESS
};

int yorha_dbg_init();
void display_idt_gates();

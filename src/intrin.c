#include "../include/intrin.h"


void __sidt(IDTR* idtr)
{
    __asm__ volatile("sidt %0" :: "m"(*idtr));
}
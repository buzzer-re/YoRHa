#include "../include/kernel.h"

void (*critical_enter)();
void (*critical_exit)();
int (*kprintf)(const char *format, ...) = (int(*)(const char *, ...)) NULL;
uint8_t* kernel_base = 0;

//
// Init kernel pointers
//
//
// Only for fw 9.0, for now...
// 
void init_kernel()
{
    kernel_base = &((uint8_t *)__readmsr(0xC0000082))[-0x000001C0];  
	
    critical_enter  = (void(*)()) &kernel_base[0x002C1980]; // critical_enter offset, from mira
    critical_exit   = (void(*)()) &kernel_base[0x002C19A0]; // critical_exit offset, from mira
	kprintf = (int(*)(const char *, ...)) ( (void*)((uint8_t *) (&kernel_base[0x000B7A30])));

    kprintf("[+] Kernel base: %p [+]\n", kernel_base);
}


//
// Make the thread enter in a critical region
// Clear IF bit, to avoid maskable interruptions manipulating system structures/data
// Clear the wp bit
//
void enable_safe_patch()
{
    if (!kernel_base)
    {
        init_kernel();
    }

    critical_enter();
    __asm__("cli"); 
    disable_cr0_wp();
}

//
// Disable/enable everything that was done in the enable_safe_patch
//
void disable_safe_patch()
{
    if (!kernel_base)
    {
        init_kernel();
    }

    enable_cr0_wp();
    __asm__("sti");
    critical_exit();
}


void disable_cr0_wp()
{
    __writecr0(__readcr0() & ~X86_CR0_WP);
}	

void enable_cr0_wp()
{
    __writecr0(__readcr0() | X86_CR0_WP);
}
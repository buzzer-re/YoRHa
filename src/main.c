#include "ps4.h"
#include "../include/yorha_dbg.h"

#define SYS_fork 2

int _main()
{
    initKernel();
    initLibc();
    initSysUtil();
    jailbreak();

    printf_debug("YorHa loaded!");

    //
    // detach
    //
    // int pid = getpid();
    // syscall(SYS_fork);

    // if (getpid() == pid)
    // {
    //     goto exit;
    // }
    //
    // Start kernel debugger
    //
    if (yorha_dbg_init())
    {
        printf_debug("Kernel Debugger enabled");
    }


    
exit:
    return 0;
}

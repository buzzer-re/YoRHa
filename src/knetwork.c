#include "../include/knetwork.h"


int ksocket(int domain, int type, int protocol, struct thread* td)
{
    if (!kernel_base) return 1;

    struct socket_args uap;
    int err;

    uap.domain = domain;
    uap.type = type;
    uap.protocol = protocol;
    
    err = sys_socket(td, &uap);
    if (err) return -err;

    return td->td_retval[0];
}
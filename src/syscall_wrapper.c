#include "../include/syscall_wrapper.h"

//
// Create a socket using the syscall function address directly
//
int ksocket(int domain, int type, int protocol, struct thread* td)
{
    kprintf("ksocket1\n");
    if (!kernel_base) return -1;
    kprintf("ksocket2\n");

    struct socket_args uap;
    int err;

    uap.domain = domain;
    uap.type = type;
    uap.protocol = protocol;
    kprintf("Calling sys_socket at %llx\n", ksys_socket);
    err = ksys_socket(td, &uap);
    kprintf("err: %d\n", err);

    if (err) return -err;

    return td->td_retval[0];
}

//
// Close a fd using the close syscall function address directly
//
int kclose(int socket, struct thread* td)
{
    if (!kernel_base) return 0;
    
    struct close_args uap;
    int err;
    uap.fd = socket;
    
    err = ksys_close(td, &uap);
    if (err) return -err;

    return td->td_retval[0];
}

int kbind(int socket, struct sockaddr addr, struct thread* td)
{
    if (!kernel_base) return 0;
    
    struct bind_args uap = {0};
    int err;
    
    uap.s = socket;

    err = ksys_bind(td, &uap);
    if (err) return -err;

    return td->td_retval[0];
}
#include "../include/syscall_wrapper.h"
#define CHECK_KERNEL_BASE() if (!kernel_base) return -1
#define CALL_RET() if (err) return -err; return td->td_retval[0];

//
// Create a socket using the syscall function address directly
//
int ksocket(int domain, int type, int protocol, struct thread* td)
{
    CHECK_KERNEL_BASE();

    struct socket_args uap;
    int err;

    uap.domain = domain;
    uap.type = type;
    uap.protocol = protocol;
    err = ksys_socket(td, &uap);
    CALL_RET();
}

//
// Close a fd using the close syscall function address directly
//
int kclose(int socket, struct thread* td)
{
    CHECK_KERNEL_BASE();
    
    struct close_args uap;
    int err;
    uap.fd = socket;
    
    err = ksys_close(td, &uap);
    CALL_RET();
}

//
// sys_bind wrapper call
//
int kbind(int socket, struct sockaddr* addr, size_t addrlen, struct thread* td)
{
    CHECK_KERNEL_BASE();
    
    struct bind_args uap;
    int err;
    
    uap.namelen = addrlen;
    uap.name = (char*) addr;
    uap.s = socket;
    

    err = ksys_bind(td, &uap);
    CALL_RET();
}

//
// sys_recvfrom wrapper call
//
int krecvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, size_t *addrlen, struct thread* td)
{
    CHECK_KERNEL_BASE();

    struct recvfrom_args uap;
    int err;
    uap.buf = buf;
    uap.s = sockfd;
    uap.len = len;
    uap.flags = flags;
    uap.from = src_addr;
    uap.fromlenaddr = (socklen_t*) addrlen;

    err = ksys_recvfrom(td, &uap);

    CALL_RET();
}

//
// sys_listen wrapper call
//
int klisten(int socket, int backlog, struct thread* td)
{
    CHECK_KERNEL_BASE();

    struct listen_args uap;
    int err;

    uap.s = socket;
    uap.backlog = backlog;

    err = ksys_listen(td, &uap);

    CALL_RET();
}

//
// sys_accept wrapper call
//
int kaccept(int sockfd, struct sockaddr *addr, socklen_t *addrlen, struct thread* td)
{
    CHECK_KERNEL_BASE();
    
    struct accept_args uap;
    int err;

    uap.name = addr;
    uap.anamelen = addrlen;
    uap.s = sockfd;

    err = ksys_accept(td, &uap);

    CALL_RET();
}


int kread(int fd, void *buf, size_t count, struct thread* td)
{
    CHECK_KERNEL_BASE();

    struct read_args uap;
    int err;

    uap.buf = buf;
    uap.fd = fd;
    uap.nbyte = count;

    err = ksys_read(td, &uap);

    CALL_RET();    
}


int ksendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen, struct thread* td)
{
    CHECK_KERNEL_BASE();
    
    struct sendto_args uap;
    int err;
    
    uap.s = sockfd;
    uap.buf = (char*) buf;
    uap.flags = flags;
    uap.len = len;
    uap.to = (char*) dest_addr;
    uap.tolen = addrlen;

    err = ksys_sendto(td, &uap);

    CALL_RET();
}
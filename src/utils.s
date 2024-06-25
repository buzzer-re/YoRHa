.intel_syntax noprefix

.global __get_rsp
.global __get_rip


__get_rsp:
    mov rax, rsp
    ret

__get_rip:
    mov rax, [rsp]
    ret
    
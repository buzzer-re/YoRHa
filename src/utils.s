.intel_syntax noprefix

.global __get_rsp
.global __get_rip
.global memcpy
.global memset


__get_rsp:
    mov rax, rsp
    ret

__get_rip:
    mov rax, [rsp]
    ret

memcpy:
	mov rax, rdi
	mov rcx, rdx
	rep movsb
	ret

memset:
    push rdi
    mov     r8,     rdi 
    mov     rax,    rsi 
    mov     rcx,    rdx 
    rep     stosb
    pop rdi
    mov rax, rdi
    ret

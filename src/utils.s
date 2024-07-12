.intel_syntax noprefix

.global __get_rsp
.global __get_rip
.global memcpy


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



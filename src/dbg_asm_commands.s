.intel_syntax noprefix

.section .text
.global __debugbreak

__debugbreak:
    int3;
    ret

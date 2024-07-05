.intel_syntax noprefix

.global __debugbreak

__debugbreak:
    int3;
    ret
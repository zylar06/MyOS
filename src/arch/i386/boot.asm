
section .multiboot
align 4
dd 0x1BADB002
dd 0x00
dd -(0x1BADB002)

section .bss
align 16
stack_bottom:
    resb 16384
global stack_top
stack_top:

section .text
global _start
extern kernel_main

_start:
    cld
    mov esp,stack_top
    push ebx                    ; multiboot information address
    push eax                    ; multiboot magic
    call kernel_main

hang:
    hlt
    jmp hang

section .text
global syscall_entry
extern syscall_dispatch

syscall_entry:
    pusha
    push esp
    call syscall_dispatch
    add esp, 4
    popa
    iretd

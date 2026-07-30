section .text

global idt_load
%assign i 0
%rep 32
global isr%+i
%assign i i + 1
%endrep
%assign i 0
%rep 16
global irq%+i
%assign i i + 1
%endrep

extern exception_handler
extern irq_handler

idt_load:
    mov eax, [esp + 4]
    lidt [eax]
    ret

; Exceptions without a CPU-pushed error code get a synthetic zero so the
; common handler has one stack layout for every exception.
%macro ISR_NOERR 1
isr%1:
    push dword 0
    push dword %1
    jmp isr_common
%endmacro

; The CPU already pushed an error code for these exceptions.
%macro ISR_ERR 1
isr%1:
    push dword %1
    jmp isr_common
%endmacro

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR   17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_ERR   21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_ERR   29
ISR_ERR   30
ISR_NOERR 31

isr_common:
    pusha
    push esp
    call exception_handler
    add esp, 4
    popa
    add esp, 8                 ; vector + error code
    iretd

%macro IRQ 1
irq%1:
    push dword %1
    jmp irq_common
%endmacro

IRQ 0
IRQ 1
IRQ 2
IRQ 3
IRQ 4
IRQ 5
IRQ 6
IRQ 7
IRQ 8
IRQ 9
IRQ 10
IRQ 11
IRQ 12
IRQ 13
IRQ 14
IRQ 15

irq_common:
    pusha
    push esp
    call irq_handler
    add esp, 4
    popa
    add esp, 4                 ; IRQ number
    iretd

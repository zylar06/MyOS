section .text
global context_switch

; void context_switch(uint32_t *old_esp, uint32_t new_esp)
context_switch:
    pushad
    mov edx, [esp + 36]
    mov [edx], esp
    mov esp, [esp + 40]
    popad
    ret

section .text
global enter_user_mode

enter_user_mode:
    mov eax, [esp + 4]
    mov edx, [esp + 8]
    push dword 0x23
    push edx
    push dword 0x202
    push dword 0x1B
    push eax
    iretd

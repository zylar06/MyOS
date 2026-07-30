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

section .usertext
bits 32
global user_program_start
global user_program_end

user_program_start:
    call .get_ip
.get_ip:
    pop ecx
    add ecx, user_message - .get_ip
    mov eax, 1
    mov ebx, 1
    mov edx, user_message_end - user_message
    int 0x80
    call .get_path
.get_path:
    pop ecx
    add ecx, user_path - .get_path
    mov eax, 2
    mov ebx, ecx
    int 0x80
    mov esi, eax
    call .get_buffer
.get_buffer:
    pop ecx
    add ecx, user_buffer - .get_buffer
    mov eax, 3
    mov ebx, esi
    mov edx, user_buffer_end - user_buffer
    int 0x80
    mov edx, eax
    mov eax, 1
    mov ebx, 1
    int 0x80
    mov eax, 4
    mov ebx, esi
    int 0x80
.halt:
    jmp .halt

user_message db "Hello from ring 3 user mode!", 10
user_message_end:
user_path db "/hello.txt", 0
user_buffer times 64 db 0
user_buffer_end:
user_program_end:

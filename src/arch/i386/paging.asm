section .text
global paging_enable

paging_enable:
    mov eax, [esp + 4]
    mov cr3, eax

    mov eax, cr0
    or eax, 0x80000000          ; CR0.PG: enable paging
    mov cr0, eax
    ret

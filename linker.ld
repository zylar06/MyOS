ENTRY(_start)

SECTIONS
{
    . = 0x100000;

    .multiboot :
    {
        *(.multiboot)
    }

    .text :
    {
        *(.text)
    }

    .usertext ALIGN(0x1000) :
    {
        __user_program_start = .;
        *(.usertext)
        __user_program_end = .;
    }

    .rodata :
    {
        *(.rodata)
    }

    .data :
    {
        *(.data)
    }

    .bss :
    {
        *(COMMON)
        *(.bss)
    }

    __kernel_end = .;
}

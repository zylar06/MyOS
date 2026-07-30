CC := i686-elf-gcc
LD := i686-elf-ld
NASM := nasm
QEMU := qemu-system-i386

CFLAGS := -m32 -std=gnu11 -ffreestanding -fno-pie -fno-stack-protector \
           -Wall -Wextra -Wpedantic
LDFLAGS := -m elf_i386 -T linker.ld

C_SOURCES := \
    src/kernel/main.c \
    src/arch/i386/gdt.c \
    src/arch/i386/idt.c \
    src/arch/i386/pic.c \
    src/mm/physical.c \
    src/mm/paging.c \
    src/mm/heap.c \
    src/drivers/timer.c \
    src/drivers/keyboard.c \
    src/sched/process.c \
    src/fs/ramfs.c \
    src/syscall/syscall.c

ASM_SOURCES := \
    src/arch/i386/boot.asm \
    src/arch/i386/gdt.asm \
    src/arch/i386/idt.asm \
    src/arch/i386/paging.asm \
    src/arch/i386/process.asm \
    src/arch/i386/user.asm \
    src/arch/i386/syscall.asm \
    src/user/init.asm

C_OBJECTS := $(C_SOURCES:src/%.c=build/c/%.o)
ASM_OBJECTS := $(ASM_SOURCES:src/%.asm=build/asm/%.o)
OBJECTS := $(C_OBJECTS) $(ASM_OBJECTS)

.PHONY: all run clean

all: kernel.bin

kernel.bin: $(OBJECTS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS)

build/c/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@

build/asm/%.o: src/%.asm
	mkdir -p $(dir $@)
	$(NASM) -f elf32 $< -o $@

run: kernel.bin
	$(QEMU) -kernel $<

clean:
	rm -rf build kernel.bin

all: kernel.bin

.PHONY: all run clean

boot.o: boot.asm
	nasm -f elf32 boot.asm -o boot.o

gdt_asm.o: gdt.asm
	nasm -f elf32 gdt.asm -o gdt_asm.o

kernel.o: kernel.c
	i686-elf-gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -c kernel.c -o kernel.o

gdt.o: gdt.c
	i686-elf-gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -c gdt.c -o gdt.o

idt_asm.o: idt.asm
	nasm -f elf32 idt.asm -o idt_asm.o

idt.o: idt.c
	i686-elf-gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -c idt.c -o idt.o

pic.o: pic.c
	i686-elf-gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -c pic.c -o pic.o

timer.o: timer.c
	i686-elf-gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -c timer.c -o timer.o

keyboard.o: keyboard.c
	i686-elf-gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -c keyboard.c -o keyboard.o

physical_memory.o: physical_memory.c
	i686-elf-gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -c physical_memory.c -o physical_memory.o

paging_asm.o: paging.asm
	nasm -f elf32 paging.asm -o paging_asm.o

paging.o: paging.c
	i686-elf-gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -c paging.c -o paging.o

kernel_heap.o: kernel_heap.c
	i686-elf-gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -c kernel_heap.c -o kernel_heap.o

kernel.bin: boot.o kernel.o gdt.o gdt_asm.o idt.o idt_asm.o pic.o timer.o keyboard.o physical_memory.o paging.o paging_asm.o kernel_heap.o linker.ld
	i686-elf-ld -m elf_i386 -T linker.ld -o kernel.bin boot.o kernel.o gdt.o gdt_asm.o idt.o idt_asm.o pic.o timer.o keyboard.o physical_memory.o paging.o paging_asm.o kernel_heap.o

run: kernel.bin
	qemu-system-i386 -kernel kernel.bin

clean:
	rm -f boot.o kernel.o gdt.o gdt_asm.o idt.o idt_asm.o pic.o timer.o keyboard.o physical_memory.o paging.o paging_asm.o kernel_heap.o kernel.bin

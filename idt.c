typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

struct idt_entry
{
    uint16_t base_low;
    uint16_t selector;
    uint8_t always_zero;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_pointer
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct exception_registers
{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t saved_esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t vector;
    uint32_t error_code;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t useresp;
    uint32_t ss;
};

static struct idt_entry idt[256];
static struct idt_pointer idt_ptr;

extern void idt_load(uint32_t idt_ptr_address);

#define DECLARE_ISR(n) extern void isr##n(void)
DECLARE_ISR(0);  DECLARE_ISR(1);  DECLARE_ISR(2);  DECLARE_ISR(3);
DECLARE_ISR(4);  DECLARE_ISR(5);  DECLARE_ISR(6);  DECLARE_ISR(7);
DECLARE_ISR(8);  DECLARE_ISR(9);  DECLARE_ISR(10); DECLARE_ISR(11);
DECLARE_ISR(12); DECLARE_ISR(13); DECLARE_ISR(14); DECLARE_ISR(15);
DECLARE_ISR(16); DECLARE_ISR(17); DECLARE_ISR(18); DECLARE_ISR(19);
DECLARE_ISR(20); DECLARE_ISR(21); DECLARE_ISR(22); DECLARE_ISR(23);
DECLARE_ISR(24); DECLARE_ISR(25); DECLARE_ISR(26); DECLARE_ISR(27);
DECLARE_ISR(28); DECLARE_ISR(29); DECLARE_ISR(30); DECLARE_ISR(31);

#define DECLARE_IRQ(n) extern void irq##n(void)
DECLARE_IRQ(0);  DECLARE_IRQ(1);  DECLARE_IRQ(2);  DECLARE_IRQ(3);
DECLARE_IRQ(4);  DECLARE_IRQ(5);  DECLARE_IRQ(6);  DECLARE_IRQ(7);
DECLARE_IRQ(8);  DECLARE_IRQ(9);  DECLARE_IRQ(10); DECLARE_IRQ(11);
DECLARE_IRQ(12); DECLARE_IRQ(13); DECLARE_IRQ(14); DECLARE_IRQ(15);

extern void terminal_write(const char *message);
extern void terminal_write_uint(unsigned int value);
extern void terminal_write_hex(unsigned int value);

static void idt_set_gate(uint8_t number, uint32_t base)
{
    idt[number].base_low = base & 0xFFFF;
    idt[number].base_high = (base >> 16) & 0xFFFF;
    idt[number].selector = 0x08;
    idt[number].always_zero = 0;
    idt[number].flags = 0x8E; // present, ring 0, 32-bit interrupt gate
}

void idt_init(void)
{
    for (int i = 0; i < 256; i++)
    {
        idt[i].base_low = 0;
        idt[i].base_high = 0;
        idt[i].selector = 0;
        idt[i].always_zero = 0;
        idt[i].flags = 0;
    }

    void (*const handlers[32])(void) = {
        isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7,
        isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15,
        isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23,
        isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
    };

    for (int i = 0; i < 32; i++)
    {
        idt_set_gate((uint8_t)i, (uint32_t)handlers[i]);
    }

    void (*const irq_handlers[16])(void) = {
        irq0, irq1, irq2, irq3, irq4, irq5, irq6, irq7,
        irq8, irq9, irq10, irq11, irq12, irq13, irq14, irq15
    };

    for (int i = 0; i < 16; i++)
    {
        idt_set_gate((uint8_t)(32 + i), (uint32_t)irq_handlers[i]);
    }

    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base = (uint32_t)&idt[0];
    idt_load((uint32_t)&idt_ptr);
}

__attribute__((noreturn))
void exception_handler(struct exception_registers *registers)
{
    terminal_write("CPU exception #");
    terminal_write_uint(registers->vector);
    terminal_write(" error=");
    terminal_write_hex(registers->error_code);
    terminal_write(" eip=");
    terminal_write_hex(registers->eip);
    terminal_write("\nKernel halted.\n");

    for (;;)
    {
        __asm__ volatile("cli; hlt");
    }
}

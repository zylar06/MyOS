typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#define PIT_COMMAND 0x43
#define PIT_CHANNEL0 0x40
#define PIT_BASE_FREQUENCY 1193182
#define TIMER_FREQUENCY 100

struct irq_registers
{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t saved_esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t irq;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t useresp;
    uint32_t ss;
};

static volatile uint32_t ticks;

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

extern void pic_unmask_irq(uint8_t irq);
extern void pic_send_eoi(uint8_t irq);
extern void terminal_write(const char *message);
extern void terminal_write_uint(unsigned int value);
extern void keyboard_irq_handler(void);

void timer_init(void)
{
    uint16_t divisor = PIT_BASE_FREQUENCY / TIMER_FREQUENCY;

    // Channel 0, low/high divisor bytes, square-wave mode.
    outb(PIT_COMMAND, 0x36);
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, divisor >> 8);

    pic_unmask_irq(0);
    __asm__ volatile("sti");
}

void irq_handler(struct irq_registers *registers)
{
    if (registers->irq == 0)
    {
        ticks++;

        if (ticks % TIMER_FREQUENCY == 0)
        {
            terminal_write("Timer ticks: ");
            terminal_write_uint(ticks);
            terminal_write("\n");
        }
    }
    else if (registers->irq == 1)
    {
        keyboard_irq_handler();
    }

    pic_send_eoi((uint8_t)registers->irq);
}

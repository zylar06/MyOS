typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define PIC_INIT     0x11
#define PIC_8086     0x01

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline void io_wait(void)
{
    outb(0x80, 0);
}

void pic_remap(void)
{
    // Start initialization and wait between each command for slow hardware.
    outb(PIC1_COMMAND, PIC_INIT);
    io_wait();
    outb(PIC2_COMMAND, PIC_INIT);
    io_wait();

    // Avoid the CPU exception vectors 0x00-0x1F.
    outb(PIC1_DATA, 0x20);
    io_wait();
    outb(PIC2_DATA, 0x28);
    io_wait();

    // The slave is connected to IRQ2 on the master.
    outb(PIC1_DATA, 0x04);
    io_wait();
    outb(PIC2_DATA, 0x02);
    io_wait();

    // Use 8086/88 mode.
    outb(PIC1_DATA, PIC_8086);
    io_wait();
    outb(PIC2_DATA, PIC_8086);
    io_wait();

    // Keep all hardware IRQs masked until their handlers are installed.
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}

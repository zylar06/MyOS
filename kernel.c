#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((volatile char *)0xB8000)

extern void gdt_init(void);
extern void idt_init(void);
extern void pic_remap(void);
extern void timer_init(void);
extern void keyboard_init(void);
extern void physical_memory_init(unsigned int magic, unsigned int info_address);

static int row = 0;
static int column = 0;
static char color = 0x07;

void terminal_clear(void)
{
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
    {
        VGA_MEMORY[i * 2] = ' ';
        VGA_MEMORY[i * 2 + 1] = color;
    }

    row = 0;
    column = 0;
}
void terminal_scroll(void)
{
    // 第1～24行复制到第0～23行
    for (int row_index = 1; row_index < VGA_HEIGHT; row_index++)
    {
        for (int column_index = 0;
             column_index < VGA_WIDTH;
             column_index++)
        {
            int from = (row_index * VGA_WIDTH + column_index) * 2;
            int to = ((row_index - 1) * VGA_WIDTH + column_index) * 2;

            VGA_MEMORY[to] = VGA_MEMORY[from];
            VGA_MEMORY[to + 1] = VGA_MEMORY[from + 1];
        }
    }

    // 清空最后一行
    for (int column_index = 0;
         column_index < VGA_WIDTH;
         column_index++)
    {
        int index =
            ((VGA_HEIGHT - 1) * VGA_WIDTH + column_index) * 2;

        VGA_MEMORY[index] = ' ';
        VGA_MEMORY[index + 1] = color;
    }

    row = VGA_HEIGHT - 1;
}
void terminal_putchar(char c)
{
    if (c == '\n')
    {
        row++;
        column = 0;

        if (row >= VGA_HEIGHT)
        {
            terminal_scroll();
        }

        return;
    }

    int index = (row * VGA_WIDTH + column) * 2;

    VGA_MEMORY[index] = c;
    VGA_MEMORY[index + 1] = color;

    column++;

    if (column >= VGA_WIDTH)
    {
        column = 0;
        row++;

        if (row >= VGA_HEIGHT)
        {
            terminal_scroll();
        }
    }
}
void terminal_write(const char *message)
{
    for (int i = 0; message[i] != '\0'; i++)
    {
        terminal_putchar(message[i]);
    }
}
void terminal_write_uint(unsigned int value)
{
    char buffer[10];
    int length = 0;

    if (value == 0)
    {
        terminal_putchar('0');
        return;
    }

    while (value > 0)
    {
        buffer[length] = '0' + value % 10;
        value = value / 10;
        length++;
    }

    for (int i = length - 1; i >= 0; i--)
    {
        terminal_putchar(buffer[i]);
    }
}
void terminal_write_hex(unsigned int value)
{
    const char *digits = "0123456789ABCDEF";

    terminal_write("0x");

    for (int shift = 28; shift >= 0; shift -= 4)
    {
        int digit = (value >> shift) & 0xF;
        terminal_putchar(digits[digit]);
    }
}
void kernel_main(unsigned int magic, unsigned int info_address)
{
    terminal_clear();

    terminal_write("Initializing GDT...\n");
    gdt_init();
    terminal_write("GDT initialized successfully\n");

    terminal_write("Initializing IDT...\n");
    idt_init();
    terminal_write("IDT initialized successfully\n");

    terminal_write("Remapping PIC...\n");
    pic_remap();
    terminal_write("PIC remapped successfully\n");

    terminal_write("Initializing timer...\n");
    timer_init();
    terminal_write("Timer initialized successfully\n");

    keyboard_init();
    terminal_write("Keyboard initialized successfully\n");

    physical_memory_init(magic, info_address);
}

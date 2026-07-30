typedef unsigned int uint32_t;

#define SYS_WRITE 1
#define SYS_OPEN  2
#define SYS_READ  3
#define SYS_CLOSE 4
#define SYS_EXIT  5
#define SYS_YIELD 6

#define USER_START 0x00400000U
#define USER_END   0x00900000U

struct syscall_registers
{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t saved_esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t useresp;
    uint32_t ss;
};

extern void terminal_putchar(char character);
extern void process_exit(void);
extern void process_yield(void);
extern int ramfs_open(const char *path);
extern int ramfs_read(int fd, char *buffer, uint32_t length);
extern int ramfs_close(int fd);

static int user_range_valid(uint32_t address, uint32_t length)
{
    if (address < USER_START || address >= USER_END)
    {
        return 0;
    }
    return length <= USER_END - address;
}

static uint32_t syscall_write(uint32_t fd, uint32_t address, uint32_t length)
{
    if (fd != 1 || !user_range_valid(address, length))
    {
        return (uint32_t)-1;
    }

    for (uint32_t i = 0; i < length; i++)
    {
        terminal_putchar(((const char *)address)[i]);
    }
    return length;
}

static int user_string_valid(uint32_t address)
{
    if (address < USER_START || address >= USER_END)
    {
        return 0;
    }
    for (uint32_t i = 0; i < 128 && address + i < USER_END; i++)
    {
        if (((const char *)address)[i] == '\0')
        {
            return 1;
        }
    }
    return 0;
}

void syscall_dispatch(struct syscall_registers *registers)
{
    switch (registers->eax)
    {
        case SYS_WRITE:
            registers->eax = syscall_write(
                registers->ebx, registers->ecx, registers->edx);
            break;
        case SYS_OPEN:
            registers->eax = user_string_valid(registers->ebx)
                ? (uint32_t)ramfs_open((const char *)registers->ebx)
                : (uint32_t)-1;
            break;
        case SYS_READ:
        {
            static char buffer[128];
            uint32_t length = registers->edx;
            if (length > sizeof(buffer))
            {
                length = sizeof(buffer);
            }
            if (!user_range_valid(registers->ecx, length))
            {
                registers->eax = (uint32_t)-1;
                break;
            }
            int result = ramfs_read((int)registers->ebx, buffer, length);
            if (result >= 0)
            {
                for (int i = 0; i < result; i++)
                {
                    ((char *)registers->ecx)[i] = buffer[i];
                }
            }
            registers->eax = (uint32_t)result;
            break;
        }
        case SYS_CLOSE:
            registers->eax = (uint32_t)ramfs_close((int)registers->ebx);
            break;
        case SYS_EXIT:
            process_exit();
            break;
        case SYS_YIELD:
            process_yield();
            registers->eax = 0;
            break;
        default:
            registers->eax = (uint32_t)-1;
            break;
    }
}

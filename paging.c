typedef unsigned int uint32_t;

#define PAGE_SIZE 4096U
#define PAGE_PRESENT 0x001U
#define PAGE_WRITABLE 0x002U

static uint32_t page_directory[1024] __attribute__((aligned(PAGE_SIZE)));
static uint32_t page_tables[4][1024] __attribute__((aligned(PAGE_SIZE)));

extern void paging_enable(uint32_t directory_address);
extern void terminal_write(const char *message);

void paging_init(void)
{
    for (uint32_t i = 0; i < 1024; i++)
    {
        page_directory[i] = 0;
    }

    for (uint32_t table = 0; table < 4; table++)
    {
        for (uint32_t entry = 0; entry < 1024; entry++)
        {
            uint32_t page = table * 1024 + entry;
            page_tables[table][entry] = page * PAGE_SIZE |
                PAGE_PRESENT | PAGE_WRITABLE;
        }

        page_directory[table] = (uint32_t)&page_tables[table][0] |
            PAGE_PRESENT | PAGE_WRITABLE;
    }

    paging_enable((uint32_t)&page_directory[0]);
    terminal_write("Paging enabled: identity-mapped first 16 MiB\n");
}

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002
#define MULTIBOOT_INFO_MEMORY_MAP  (1 << 6)
#define MULTIBOOT_MEMORY_AVAILABLE 1

#define PAGE_SIZE 4096U
#define MAX_PHYSICAL_PAGES 1048576U // 4 GiB / 4 KiB
#define BITMAP_BYTES (MAX_PHYSICAL_PAGES / 8)

struct multiboot_info
{
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
} __attribute__((packed));

struct multiboot_mmap_entry
{
    uint32_t size;
    uint64_t base;
    uint64_t length;
    uint32_t type;
} __attribute__((packed));

static uint8_t page_bitmap[BITMAP_BYTES];
static uint32_t free_pages;

extern char __kernel_end;
extern void terminal_write(const char *message);
extern void terminal_write_uint(unsigned int value);
extern void terminal_write_hex(unsigned int value);

static void mark_page_used(uint32_t page)
{
    uint8_t mask = (uint8_t)(1U << (page & 7));

    if ((page_bitmap[page >> 3] & mask) == 0)
    {
        page_bitmap[page >> 3] |= mask;
        if (free_pages > 0)
        {
            free_pages--;
        }
    }
}

static void mark_page_free(uint32_t page)
{
    uint8_t mask = (uint8_t)(1U << (page & 7));

    if ((page_bitmap[page >> 3] & mask) != 0)
    {
        page_bitmap[page >> 3] &= (uint8_t)~mask;
        free_pages++;
    }
}

static void mark_range_free(uint64_t base, uint64_t length)
{
    uint64_t end = base + length;
    uint64_t first_page = (base + PAGE_SIZE - 1) / PAGE_SIZE;
    uint64_t last_page = end / PAGE_SIZE;

    if (first_page >= MAX_PHYSICAL_PAGES)
    {
        return;
    }
    if (last_page > MAX_PHYSICAL_PAGES)
    {
        last_page = MAX_PHYSICAL_PAGES;
    }

    for (uint64_t page = first_page; page < last_page; page++)
    {
        mark_page_free((uint32_t)page);
    }
}

void physical_memory_init(uint32_t magic, uint32_t info_address)
{
    for (uint32_t i = 0; i < BITMAP_BYTES; i++)
    {
        page_bitmap[i] = 0xFF;
    }
    free_pages = 0;

    if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
    {
        terminal_write("Invalid Multiboot magic: ");
        terminal_write_hex(magic);
        terminal_write("\n");
        return;
    }

    struct multiboot_info *info = (struct multiboot_info *)info_address;
    if ((info->flags & MULTIBOOT_INFO_MEMORY_MAP) == 0)
    {
        terminal_write("Multiboot memory map unavailable\n");
        return;
    }

    uint32_t offset = 0;
    while (offset < info->mmap_length)
    {
        struct multiboot_mmap_entry *entry =
            (struct multiboot_mmap_entry *)(info->mmap_addr + offset);

        if (entry->type == MULTIBOOT_MEMORY_AVAILABLE)
        {
            mark_range_free(entry->base, entry->length);
        }

        offset += entry->size + sizeof(entry->size);
    }

    // Keep low memory and the kernel image unavailable to allocations.
    for (uint32_t page = 0; page < 0x100000 / PAGE_SIZE; page++)
    {
        mark_page_used(page);
    }

    uint32_t kernel_end = (uint32_t)&__kernel_end;
    for (uint32_t page = 0x100000 / PAGE_SIZE;
         page < (kernel_end + PAGE_SIZE - 1) / PAGE_SIZE;
         page++)
    {
        mark_page_used(page);
    }

    terminal_write("Free physical pages: ");
    terminal_write_uint(free_pages);
    terminal_write("\n");
}

void *physical_page_alloc(void)
{
    for (uint32_t page = 0; page < MAX_PHYSICAL_PAGES; page++)
    {
        if ((page_bitmap[page >> 3] & (1U << (page & 7))) == 0)
        {
            mark_page_used(page);
            return (void *)(page * PAGE_SIZE);
        }
    }

    return (void *)0;
}

void physical_page_free(void *address)
{
    uint32_t physical_address = (uint32_t)address;

    if ((physical_address % PAGE_SIZE) == 0 &&
        physical_address / PAGE_SIZE < MAX_PHYSICAL_PAGES)
    {
        mark_page_free(physical_address / PAGE_SIZE);
    }
}

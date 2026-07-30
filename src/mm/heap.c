typedef unsigned int uint32_t;

#define PAGE_SIZE 4096U
#define HEAP_BASE 0x01000000U

static uint32_t heap_mapped_pages;
static uint32_t heap_offset;

extern void *physical_page_alloc(void);
extern int paging_map_page(uint32_t virtual_address, uint32_t physical_address);
extern void terminal_write(const char *message);

void heap_init(void)
{
    heap_mapped_pages = 0;
    heap_offset = 0;
    terminal_write("Kernel heap ready at 0x01000000\n");
}

static int heap_add_page(void)
{
    void *physical_page = physical_page_alloc();
    if (physical_page == (void *)0)
    {
        return 0;
    }

    uint32_t virtual_address = HEAP_BASE + heap_mapped_pages * PAGE_SIZE;
    if (!paging_map_page(virtual_address, (uint32_t)physical_page))
    {
        return 0;
    }

    heap_mapped_pages++;
    return 1;
}

void *kmalloc(uint32_t size)
{
    if (size == 0)
    {
        return (void *)0;
    }

    size = (size + 7) & ~7U;
    uint32_t pages_needed = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    if (pages_needed == 1 && heap_mapped_pages != 0 &&
        heap_offset + size <= PAGE_SIZE)
    {
        void *address = (void *)(HEAP_BASE +
            (heap_mapped_pages - 1) * PAGE_SIZE + heap_offset);
        heap_offset += size;
        return address;
    }

    heap_offset = 0;
    for (uint32_t page = 0; page < pages_needed; page++)
    {
        if (!heap_add_page())
        {
            return (void *)0;
        }
    }

    void *address = (void *)(HEAP_BASE +
        (heap_mapped_pages - pages_needed) * PAGE_SIZE);
    heap_offset = size % PAGE_SIZE;
    return address;
}

// The bump allocator does not reclaim memory yet.
void kfree(void *address)
{
    (void)address;
}

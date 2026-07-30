typedef unsigned int uint32_t;

#define MAX_PROCESSES 4
#define PROCESS_STACK_SIZE 4096

enum process_state
{
    PROCESS_UNUSED,
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_ZOMBIE
};

struct process
{
    uint32_t esp;
    void (*entry)(void);
    enum process_state state;
    uint32_t id;
};

static struct process processes[MAX_PROCESSES];
static unsigned char process_stacks[MAX_PROCESSES][PROCESS_STACK_SIZE]
    __attribute__((aligned(16)));
static uint32_t current_process;
static uint32_t process_count;

extern void context_switch(uint32_t *old_esp, uint32_t new_esp);
extern void terminal_write(const char *message);

static void process_bootstrap(void);

void process_init(void)
{
    for (uint32_t i = 0; i < MAX_PROCESSES; i++)
    {
        processes[i].state = PROCESS_UNUSED;
        processes[i].id = i;
        processes[i].esp = 0;
        processes[i].entry = (void (*)(void))0;
    }

    processes[0].state = PROCESS_RUNNING;
    current_process = 0;
    process_count = 1;
}

uint32_t process_create(void (*entry)(void))
{
    if (entry == (void (*)(void))0 || process_count >= MAX_PROCESSES)
    {
        return (uint32_t)-1;
    }

    uint32_t id = 0;
    while (processes[id].state != PROCESS_UNUSED)
    {
        id++;
    }

    uint32_t *stack = (uint32_t *)&process_stacks[id][PROCESS_STACK_SIZE];
    // ret will consume this address after popad restores the registers.
    stack -= 1;
    *stack = (uint32_t)process_bootstrap;
    stack -= 8;
    for (uint32_t i = 0; i < 8; i++)
    {
        stack[i] = 0;
    }

    // context_switch starts with popad, so ESP points at the register area.
    processes[id].esp = (uint32_t)stack;
    processes[id].entry = entry;
    processes[id].state = PROCESS_READY;
    process_count++;
    return id;
}

static uint32_t find_next_process(void)
{
    for (uint32_t step = 1; step < MAX_PROCESSES; step++)
    {
        uint32_t id = (current_process + step) % MAX_PROCESSES;
        if (processes[id].state == PROCESS_READY)
        {
            return id;
        }
    }

    return current_process;
}

void process_yield(void)
{
    uint32_t next = find_next_process();
    if (next == current_process)
    {
        return;
    }

    uint32_t previous = current_process;
    processes[previous].state = PROCESS_READY;
    processes[next].state = PROCESS_RUNNING;
    current_process = next;
    context_switch(&processes[previous].esp, processes[next].esp);
}

static void process_bootstrap(void)
{
    processes[current_process].entry();
    processes[current_process].state = PROCESS_ZOMBIE;
    process_count--;
    process_yield();

    for (;;)
    {
        __asm__ volatile("hlt");
    }
}

void process_demo_task(void)
{
    terminal_write("Process task is running\n");
}

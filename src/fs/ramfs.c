typedef unsigned int uint32_t;

#define RAMFS_FD_COUNT 8

static const char hello_file[] =
    "Hello from ramfs!\n";
static int fd_used[RAMFS_FD_COUNT];
static uint32_t fd_offsets[RAMFS_FD_COUNT];

void ramfs_init(void)
{
    for (uint32_t i = 0; i < RAMFS_FD_COUNT; i++)
    {
        fd_used[i] = 0;
        fd_offsets[i] = 0;
    }
}

static int path_equals_hello(const char *path)
{
    const char expected[] = "/hello.txt";
    for (uint32_t i = 0; expected[i] != '\0'; i++)
    {
        if (path[i] != expected[i])
        {
            return 0;
        }
    }
    return path[10] == '\0';
}

int ramfs_open(const char *path)
{
    if (!path_equals_hello(path))
    {
        return -1;
    }

    for (int fd = 0; fd < RAMFS_FD_COUNT; fd++)
    {
        if (!fd_used[fd])
        {
            fd_used[fd] = 1;
            fd_offsets[fd] = 0;
            return fd;
        }
    }
    return -1;
}

int ramfs_read(int fd, char *buffer, uint32_t length)
{
    if (fd < 0 || fd >= RAMFS_FD_COUNT || !fd_used[fd])
    {
        return -1;
    }

    uint32_t file_length = sizeof(hello_file) - 1;
    if (fd_offsets[fd] >= file_length)
    {
        return 0;
    }

    uint32_t remaining = file_length - fd_offsets[fd];
    if (length > remaining)
    {
        length = remaining;
    }

    for (uint32_t i = 0; i < length; i++)
    {
        buffer[i] = hello_file[fd_offsets[fd] + i];
    }
    fd_offsets[fd] += length;
    return (int)length;
}

int ramfs_close(int fd)
{
    if (fd < 0 || fd >= RAMFS_FD_COUNT || !fd_used[fd])
    {
        return -1;
    }
    fd_used[fd] = 0;
    return 0;
}

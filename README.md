# MyOS

MyOS 是一个从零开始编写的 32 位 x86 hobby OS 内核，用于学习操作系统启动流程、保护模式、中断、内存管理和任务调度等基础概念。

当前内核可以通过 Multiboot 启动，在 VGA 文本模式下输出信息，并完成 GDT 和基础 IDT 的初始化。

## 当前环境

项目面向 32 位 x86（i386）架构，使用 freestanding C 和 NASM 汇编，不依赖宿主操作系统的 C 运行时库。

需要安装以下工具：

- `make`：执行构建脚本
- `nasm`：编译汇编代码
- `i686-elf-gcc`：编译 32 位 freestanding C 代码
- `i686-elf-ld`：链接内核 ELF 文件
- `qemu-system-i386`：在模拟器中运行内核

可以使用下面的命令检查工具是否可用：

```bash
make --version
nasm -v
i686-elf-gcc --version
i686-elf-ld --version
qemu-system-i386 --version
```

## 构建和运行

在项目根目录执行：

```bash
make
```

构建成功后会生成 `kernel.bin`。使用 QEMU 运行：

```bash
make run
```

清理构建产物：

```bash
make clean
```

构建生成的 `.o` 文件和 `kernel.bin` 已通过 `.gitignore` 排除，不会提交到 Git 仓库。

## 项目目录

```text
MyOS/
├── include/              # 公共接口（持续补充）
├── src/
│   ├── arch/i386/         # 启动、GDT、IDT、PIC、分页、特权级切换
│   ├── kernel/            # 内核入口和终端输出
│   ├── mm/                # 物理内存、分页、内核堆
│   ├── drivers/           # PIT 时钟和 PS/2 键盘
│   ├── sched/             # 任务结构和上下文切换
│   ├── syscall/           # int 0x80 系统调用分发
│   ├── fs/                # ramfs 文件系统
│   └── user/              # 预置用户程序
├── build/                 # 构建生成的目标文件
├── linker.ld              # 内核和用户程序链接布局
├── Makefile               # 编译、链接、运行和清理规则
└── README.md              # 项目说明文档
```

## 文件说明

### `src/arch/i386/boot.asm`

定义 Multiboot 头，提供 `_start` 入口，建立 16 KiB 内核栈，并调用 `kernel_main`。进入 C 代码前会清除方向标志（`cld`）。

### `linker.ld`

将内核链接到物理地址 `0x100000`，并按 `.multiboot`、`.text`、`.rodata`、`.data` 和 `.bss` 安排各个段。

### `src/kernel/main.c`

实现内核入口和 VGA 文本终端，包括清屏、字符输出、字符串输出、十进制/十六进制输出以及滚屏。

### `src/arch/i386/gdt.c` / `gdt.asm`

创建内核/用户代码段、数据段和 TSS，并通过 `lgdt`/`ltr` 加载。

### `src/arch/i386/idt.c` / `idt.asm`

创建 256 项 IDT 表，注册 0～31 号 CPU 异常和 32～47 号硬件 IRQ 入口。

### `src/arch/i386/pic.c`

初始化并重映射 8259A PIC：主 PIC 映射到 `0x20`～`0x27`，从 PIC 映射到 `0x28`～`0x2F`。目前只有 IRQ0 会被解除屏蔽。

### `src/drivers/timer.c`

配置 PIT，以 100 Hz 产生时钟中断；IRQ0 每次触发时增加 `ticks`，并每 100 次在 VGA 输出一次计数，同时向 PIC 发送 EOI。

### `src/drivers/keyboard.c`

读取键盘控制器的扫描码，处理基础字母、数字、符号、Shift 和回车键，并将按键转换为 VGA 文本输出。

### `src/mm/physical.c`

读取 Multiboot 内存地图，用位图记录 4 KiB 物理页框的使用状态，保留低端内存和内核自身占用的区域，并提供基础的页框分配与释放接口。

### `src/mm/paging.c` / `src/arch/i386/paging.asm`

建立页目录和页表，将低端 16 MiB 做恒等映射，并保留 16～32 MiB 作为内核堆映射区；然后加载 `CR3` 并设置 `CR0.PG` 开启分页。

### `src/mm/heap.c`

按需从物理页框分配器取得页面，映射到 16～32 MiB 的内核虚拟地址区间，并提供 `kmalloc`/`kfree`。当前是只增长、不回收的 bump allocator。

### `src/sched/process.c` / `src/arch/i386/process.asm`

提供任务结构、独立内核栈、上下文切换，以及启动 ring 3 合成用户任务所需的 TSS 内核栈切换。目前调度仍以协作式切换为主。

### `src/syscall/syscall.c` / `src/arch/i386/syscall.asm`

通过 DPL=3 的 `int 0x80` 门分发 `write`、`open`、`read`、`close`、`exit` 和 `yield`。

### `src/fs/ramfs.c`

提供内存文件 `/hello.txt`，支持内核预置文件的打开、读取和关闭。

### `src/arch/i386/user.asm`

包含用户态进入代码和预置 ring 3 程序；用户程序会输出文本并读取 `/hello.txt`。

## 当前状态

已经完成：

- Multiboot 启动入口
- 内核栈
- VGA 文本输出
- GDT 初始化
- 256 项 IDT 表结构
- 0～31 号 CPU 异常入口
- 8259A PIC 重映射
- PIT 时钟和 IRQ0 中断
- 键盘 IRQ1 中断
- Multiboot 内存地图解析
- 4 KiB 物理页框位图分配器
- 低端 16 MiB 恒等分页映射
- 基础内核堆和 `kmalloc`
- 独立内核栈和协作式任务切换
- 用户代码段、用户数据段和 TSS
- ring 3 用户任务
- `int 0x80` 系统调用
- `write/open/read/close/exit/yield` 系统调用
- ramfs 的 `/hello.txt`

计划中的功能：

- 可回收的内核堆分配器
- 稳定的时钟抢占式任务调度
- 用户进程独立页目录
- ELF 用户程序加载器
- 磁盘文件系统和 ATA 驱动
- 用户态、系统调用和文件系统

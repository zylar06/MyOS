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
├── .gitignore    # 忽略构建生成的目标文件和内核镜像
├── Makefile      # 编译、链接、运行和清理规则
├── boot.asm      # Multiboot 入口、内核栈和启动跳转
├── linker.ld     # 内核链接脚本和内存布局
├── kernel.c      # 内核主函数和 VGA 文本终端输出
├── gdt.c         # GDT 描述符表的构造和初始化
├── gdt.asm       # 加载 GDT 并刷新代码/数据段寄存器
├── idt.c         # IDT 表、CPU 异常注册和异常信息输出
├── idt.asm       # IDT 加载函数和 0～31 号异常入口
└── README.md     # 项目说明文档
```

## 文件说明

### `boot.asm`

定义 Multiboot 头，提供 `_start` 入口，建立 16 KiB 内核栈，并调用 `kernel_main`。进入 C 代码前会清除方向标志（`cld`）。

### `linker.ld`

将内核链接到物理地址 `0x100000`，并按 `.multiboot`、`.text`、`.rodata`、`.data` 和 `.bss` 安排各个段。

### `kernel.c`

实现内核入口和 VGA 文本终端，包括清屏、字符输出、字符串输出、十进制/十六进制输出以及滚屏。

### `gdt.c` / `gdt.asm`

创建空描述符、内核代码段和内核数据段，并通过 `lgdt` 加载 GDT，随后刷新段寄存器。

### `idt.c` / `idt.asm`

创建 256 项 IDT 表。目前注册了 0～31 号 CPU 异常处理入口；异常发生时会输出异常编号、错误码和 EIP，然后停止内核。硬件 IRQ、PIC、时钟和键盘中断尚未接入。

## 当前状态

已经完成：

- Multiboot 启动入口
- 内核栈
- VGA 文本输出
- GDT 初始化
- 256 项 IDT 表结构
- 0～31 号 CPU 异常入口

计划中的功能：

- 8259 PIC 重映射
- 时钟和键盘中断
- 物理内存管理
- 分页和内核堆
- 任务切换与调度器
- 用户态、系统调用和文件系统


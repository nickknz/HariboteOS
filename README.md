# 《30天自制操作系统》学习笔记
This project aims for learning Operating System using the book《30天自制操作系统》. Because the author uses a set of non-standard tools written by himself and can only run under Windows platform, I am planning to implement this project uses NASM, GCC, Qemu and other tools (on Mac Intel/M2) to compile and run on the basis of the original code.

## Project features
- Uses GCC and NASM as the toolchain, allowing cross-platform use
- Builds the kernel based on LinkerScript
- Support several command lines (mem, clear, )
- Does not rely on the author's custom HRB file format and supports the ELF file format
<!-- - Implements simple versions of some C standard library functions (libc file) -->
- Header files are split, making the structure clearer
- Supports Chinese keyboard input.

## The flow to boot HariboteOS
1. Start BIOS.
2. BIOS reads the first 1 sector(512 bytes), which is called IPL(initial boot loader), from a floppy disk into the memory 0x7c00.
3. [ipl.asm] IPL reads 10 cylinders from a floppy disk into the memory 0x8200.
4. [asmhead.asm] OS prepares boot(setting an image mode, enabling memory access more than 1MB, moving 32 bits mode, etc.)
5. [asmhead.asm] Execute the segment on bootpack.
6. [bootpack.c and other c files] Execute haribote OS.

## Memory Map
### For ipl.asm
| Memory Range            | Description                                                           | Size     |
| ------------------------| --------------------------------------------------------------------- | -------- |
| 0x00007c00 - 0x00007dff | IPL. The first 1 sector of a floopy disk. This is the boot sector.    | 512 Bytes|
| 0x00008200 - 0x000083ff | The content of a floopy disk(10 cylinders. Except IPL.)               | 10 Cyls  |
### After bootpack.c initialization of GDT/IDT
| Memory Range            | Description                                     | Size     |
| ------------------------| ----------------------------------------------- | -------- |
| 0x00000000 - 0x000fffff | Used during boot, becomes empty afterwards      | 1 MB     |
| 0x00100000 - 0x00267fff | Used to store contents of a floppy disk         | 1440 KB  |
| 0x00268000 - 0x0026f7ff | Empty                                           | 30 KB    |
| 0x0026f800 - 0x0026ffff | Interrupt Descriptor Table (IDT)                | 2 KB     |
| 0x00270000 - 0x0027ffff | Global Descriptor Table (GDT)                   | 64 KB    |
| 0x00280000 - 0x002fffff | Boot loader program (bootpack.hrb)              | 512 KB   |
| 0x00300000 - 0x003fffff | Stack and other data                            | 1 MB     |
| 0x00400000 -            | The location for memman_alloc                    | 128MB    |

After day 9 bootpack.c line 48, the memory 0x00001000 - 0x0009e000 is freed.
### Address infomation for some structs
| Memory Range            | Description                                     | Size     |
| ------------------------| ----------------------------------------------- | -------- |
| 0x00000ff0 - 0x00000fff | ADR_BOOTINFO                                    | 16 bytes |
| 0x003c0000 -            | MEMMAN_ADDR                                     | ~32 KB   |

## Environment set up
Please use the x86_64-elf-gcc toolchain to compile on Mac. It can be installed using the command "brew install x86_64-elf-gcc x86_64-elf-binutils x86_64-elf-gdb".

To run the program, navigate to the corresponding folder and use the following command:
```sh
$ make qemu
# If the toolchain has a prefix, such as x86_64-elf-, use GCCPREFIX
$ make qemu GCCPREFIX=x86_64-elf-
```

To debug using GDB, please set DEBUG:
```sh
$ make qemu DEBUG=1
```

## Implementation of C standard library

## Important Concepts
- IDT (Global degment Descriptor Table): A data structure used by managing memory segmentation. It contains a list of segment descriptors, each of which describes a segment of memory, including its base address, size, access permissions, and other attributes. This is the simple version of initialize GDT.
``` c
// GDT (global(segment) descriptor table)
struct SegmentDescriptor {
  short limit_low, base_low;
  char base_mid, access_right;
  char limit_high, base_high;
};
struct SegmentDescriptor *gdt = (struct SegmentDescriptor *)ADR_GDT;
for (int i = 0; i <= LIMIT_GDT / 8; i++) {
    set_segmdesc(gdt + i, 0, 0, 0);
}

set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, AR_DATA32_RW);
set_segmdesc(gdt + 2, LIMIT_BOOTPACK, ADR_BOOTPACK, AR_CODE32_ER);
load_gdtr(LIMIT_GDT, ADR_GDT);
```
- GDT (Interrupt Descriptor Table): A data structure used by managing interrupts. It contains a list of interrupt descriptors, each of which describes an interrupt or exception, including its vector number, handler address, and access permissions. The IDT to handle interrupts from Keyboard and Mouse.
```c
for (int i = 0; i <= LIMIT_IDT / 8; i++) {
    set_gatedesc(idt + i, 0, 0, 0);
}
load_idtr(LIMIT_IDT, ADR_IDT);
// Set keyboard interrupt handler
set_gatedesc(idt + 0x21, (int)asm_int_handler21, 2 * 8, AR_INTGATE32);
// Set mouse interrupt handler
set_gatedesc(idt + 0x2c, (int)asm_int_handler2c, 2 * 8, AR_INTGATE32);
```

## Bug I have met and fixed
1. When I use **unsigned char** variable data to receive the return value int from fifo32_get(&fifo), an **overflow** occur 
because the range of values that can be stored in a char is -128 to 127 for a **signed char**, or 0 to 255 for an 
**unsigned char**.

## The bug have not fixed
1. In day 15, when we are using counter to test task switching (15.5 in textbook), the screen processing will be extremely slow if we are using io_sti instead of io_stihlt. 

## Project progress
- [X] day 1：Hello world
- [X] day 2：assembly and Makefile
- [X] day 3：Combine C and assembly
- [X] day 4：Show screen
- [X] day 5：GDT/IDT
- [X] day 6：Interrupt processing
- [X] day 7：FIFO and mouse control
- [X] day 8：Mouse and 32-bit mode
- [X] day 9：Memory Management
- [X] day 10：Processing overlap
- [X] day 11：Improve refresh algorithm
- [X] day 12：Timer1
- [X] day 13：Timer2
- [X] day 14：Keyboard Input
- [X] day 15：Multi-task 1
- [X] day 16：Multi-task 2
- [X] day 17：Console window
- [ ] day 18：Command line (mem, )
- [ ] day 19：应用程序
- [ ] day 20：API
- [ ] day 21：保护操作系统
- [ ] day 22：C语言应用程序（修改为ELF格式）
- [ ] 第23天：应用程序图形处理
- [ ] 第24天：窗口操作
- [ ] 第25天：更多窗口
- [ ] 第26天：窗口操作提速
- [ ] 第27天：LDT与库（未按书上处理）
- [ ] 第28天：文件操作与文字显示（不包含日文显示部分）

## Welcome to ask questions
In the process of implementation, I also encountered a lot of trouble (eg. compile error). <br>
Feel free to create an issue and ask your question, I will try my best to answer your question. <br> <br>
If you need source code or book, you can add me Wechat: Nick_kn

## Reference
- [Source code](https://github.com/hide27k/haribote-os/tree/master)


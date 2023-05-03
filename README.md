# 《30天自制操作系统》学习笔记
This project aims for learning Operating System using the book《30天自制操作系统》. Because the author uses a set of non-standard tools written by himself and can only run under Windows platform, I am planning to implement this project uses NASM, GCC, Qemu and other tools (on Mac Intel) to compile and run on the basis of the original code.

## Project features
- Uses GCC and NASM as the toolchain, allowing cross-platform use
- Builds the kernel based on LinkerScript
- Does not rely on the author's custom HRB file format and supports the ELF file format
- Implements simple versions of some C standard library functions (libc file)
- Header files are split, making the structure clearer
- Supports Chinese keyboard input.

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

## Project progress
- [X] day 1：Hello world
- [X] day 2：assembly and Makefile
- [X] day 3：Combine C and assembly
- [X] day 4：Show screen
- [ ] 第5天：GDT/IDT
- [ ] 第6天：中断处理
- [ ] 第7天：FIFO
- [ ] 第8天：鼠标与32位模式
- [ ] 第9天：内存管理
- [ ] 第10天：窗口叠加
- [ ] 第11天：窗口处理
- [ ] 第12天：定时器1
- [ ] 第13天：定时器2
- [ ] 第14天：键盘输入
- [ ] 第15天：多任务1
- [ ] 第16天：多任务2
- [ ] 第17天：命令行窗口
- [ ] 第18天：命令行命令
- [ ] 第19天：应用程序
- [ ] 第20天：API
- [ ] 第21天：保护操作系统
- [ ] 第22天：C语言应用程序（修改为ELF格式）
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


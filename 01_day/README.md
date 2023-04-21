## Abstract
Creating a "hello world" program's floppy disk image file using NASM assembly instructions such as DB, and running it through the QEMU emulator.

## NASM assembly
- **DB**(Define Byte): Write a specific byte
- **RESB**(Reserve Byte): If you want to reserve 10 bytes starting from the current address, you can write it as RESB 10
- **DW**(Define Word): word means 2 bytes
- **DD**(Define Double): souble words mean 4 bytes

## Make Image File
After implement helloos.asm, we are gonna use NASM to compile this file to image file.
```sh
$ nasm -f bin helloos.asm -o helloos.img
```
The command above consists of several main parts:

- **-f bin**: The -f option of NASM is used to specify the output file format, which can be elf, macho, win, bin, etc. Here we choose the "bin" format, which is a binary file.
- **helloos.asm**: This is the source file name(s) not specified by an option. It can include multiple source files.
- **-o helloos.img**: The "-o" option of NASM is used to specify the output file path. Here we save the output file as "helloos.img" in the current directory.

## How to run
QEMU provides emulation programs for many different architectures, such as qemu-system-i386 (32-bit x86), qemu-system-x86_64 (64-bit x86), qemu-system-arm, qemu-system-mips, etc. Since we are mainly developing an operating system for the 32-bit x86 platform, we choose qemu-system-i386 as the emulator to use.

Since we are running the OS on a simulated floppy disk, we need to use QEMU's -fda option. The -fda <file> option treats the file specified by <file> as a floppy disk image.
```sh
$ qemu-system-i386 -fda helloos.img
```

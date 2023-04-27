#!/bin/bash
nasm -f bin helloos.asm -o helloos.bin -l helloos.lst
qemu-system-i386 -fda helloos.bin
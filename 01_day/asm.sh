#!/bin/bash
nasm -f bin helloos.asm -o helloos.img
qemu-system-i386 -fda helloos.img
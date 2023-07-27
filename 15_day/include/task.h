// #include "timer.h"

#ifndef _TASK_H_
#define _TASK_H_

extern struct Timer *mt_timer;
extern int mt_tr;

// task status segment (TTS)
struct TSS32 {
    // 保存的不是寄存器的数据，而是与任务设置相关的信息，在执行任务切换的时候这 些成员不会被写入 (backlink除外，某些情况下是会被写入的)
    int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
    // 32位寄存器
    // EIP是CPU用来记录下一条需要执行的指令位于内存中哪个地址的寄存器, 因此它才被称为“指令指针”。相当于313中的program counter (pc)
    int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    // 16位寄存器
    int es, cs, ss, ds, fs, gs;
    // 与任务设置相关的信息
    int ldtr, iomap;
};

void load_tr(int tr);
void taskswitch4(void);
void taskswitch3(void);


void far_jmp(int eip, int cs);

void mt_init(void);
void mt_task_switch(void);

#endif // _TASK_H_
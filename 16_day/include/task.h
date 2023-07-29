#ifndef _TASK_H_
#define _TASK_H_

#include "timer.h"
#include "memory.h"

#define MAX_TASKS 1000 // 最大任务数量
#define TASK_GDT0 3    // 定义从GDT的几号开始分配给TSS

// #define MAX_TASKS_LV 100
// #define MAX_TASKLEVELS 10

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

struct Task {
  int sel, flags; // sel用于存放GDT的编号
//   int level, priority;
  struct TSS32 tss;
};

// struct TaskLevel {
//   int running; // 正在运行的任务数量
//   int now;     // 记录正在运行的任务
//   struct Task *tasks[MAX_TASKS_LV];
// };

// struct TaskCtl {
//   int now_lv;    // 现在活动中的level
//   int lv_change; // 在下次任务切换时是否需要改变level
//   struct TaskLevel level[MAX_TASKLEVELS];
//   struct Task tasks0[MAX_TASKS];
// };

struct TaskCtl {
    int running; /*正在运行的任务数量*/
    int now; /*这个变量用来记录当前正在运行的是哪个任务*/ 
    struct Task *tasks[MAX_TASKS];
    struct Task tasks0[MAX_TASKS];
};

extern struct TaskCtl *taskctl;
extern struct Timer *task_timer;

void load_tr(int tr);
void taskswitch4(void);
void taskswitch3(void);
void far_jmp(int eip, int cs);

void mt_init(void);
void mt_task_switch(void);

struct Task *task_init(struct MemMan *memman);
struct Task *task_alloc(void);
void task_run(struct Task *task, int level, int priority);
void task_switch(void);
void task_sleep(struct Task *task);
struct Task *task_now(void);
void task_add(struct Task *task);
void task_remove(struct Task *task);
void task_switchsub(void);

#endif // _TASK_H_
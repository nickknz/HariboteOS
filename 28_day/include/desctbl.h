#ifndef _DESCTBL_H_
#define _DESCTBL_H_

#define ADR_IDT 0x0026f800
#define LIMIT_IDT 0x000007ff
#define ADR_GDT 0x00270000
#define LIMIT_GDT 0x0000ffff

#define ADR_BOOTPACK 0x00280000
#define LIMIT_BOOTPACK 0x0007ffff

#define AR_DATA32_RW 0x4092
#define AR_CODE32_ER 0x409a
#define AR_LDT 0x0082
#define AR_TSS32 0x0089
#define AR_INTGATE32 0x008e

// GDT ar mode
// 00000000（0x00）：未使用的记录表（descriptor table）。
// 10010010（0x92）：系统专用，可读写的段。不可执行。
// 10011010（0x9a）：系统专用，可执行的段。可读不可写。
// 11110010（0xf2）：应用程序用，可读写的段。不可执行。
// 11111010（0xfa）：应用程序用，可执行的段。可读不可写。
// CPU到底是处于系统模式还是应用模式，取决于执行中的应用程序是位于访问权为
// 0x9a的段，还是位于访问权为0xfa的段。

// GDT (global(segment) descriptor table)
struct SegmentDescriptor {
  short limit_low, base_low;
  char base_mid, access_right;
  char limit_high, base_high;
};

// IDT (Interrupt descriptor table)
struct GateDescriptor {
  short offset_low, selector;
  char dw_count, access_right;
  short offset_high;
};

void init_gdtidt(void);
void set_segmdesc(struct SegmentDescriptor *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GateDescriptor *gd, int offset, int selector, int ar);

void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);

#endif // _DESCTBL_H_
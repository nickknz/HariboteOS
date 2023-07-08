# Initialize GDT and IDT
In this project, we are using segmentation for memory rather than paging.

- IDT (Global degment Descriptor Table): A data structure used by managing memory segmentation. It contains a list of segment descriptors, each of which describes a segment of memory, including its base address, size, access permissions, and other attributes. This is the simple version of initialize GDT.
``` C
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

## move mouse
The core concept of this chapter is reading 3 bytes from mouse moving. It's greate to understand how we decode the data receiving from mouse moving. The first byte indicates if click the mouse left or right. The second byte indicates x and the third one shows the y.
``` C
int mouse_decode(struct MouseDec *mdec, unsigned char dat) {
  if (mdec->phase == 0) {
    // 等待鼠标的0xfa状态
    if (dat == 0xfa) {
      mdec->phase = 1;
    }
    return 0;
  } else if (mdec->phase == 1) {
    // 等待鼠标的第一个字节
    if ((dat & 0xc8) == 0x08) {
      // 如果第一个字节正确
      mdec->buf[0] = dat;
      mdec->phase = 2;
    }
    return 0;
  } else if (mdec->phase == 2) {
    // 等待鼠标的第二个字节
    mdec->buf[1] = dat;
    mdec->phase = 3;
    return 0;
  } else if (mdec->phase == 3) {
    // 等待鼠标的第三个字节
    mdec->buf[2] = dat;
    mdec->phase = 1;

    mdec->btn = mdec->buf[0] & 0x07;
    mdec->x = mdec->buf[1];
    mdec->y = mdec->buf[2];

    if ((mdec->buf[0] & 0x10) != 0) {
      mdec->x |= 0xffffff00;
    }

    if ((mdec->buf[0] & 0x20) != 0) {
      mdec->y |= 0xffffff00;
    }

    mdec->y = -mdec->y; // 鼠标的y方向与画面符号相反

    return 1;
  }
  return -1;
}
```
## 32-bits mode
The second part is transfering to 32-bit mode.
``` assembly 
; 切换到保护模式
  LGDT  [GDTR0]           ; 设置临时GDT
  MOV   EAX, CR0
  AND   EAX, 0x7fffffff   ; 禁用分页
  OR    EAX, 0x00000001   ; 开启保护模式
  MOV   CR0, EAX
  JMP   pipelineflush
```
We got into "protected virtual address mode" after getting into protected 32-bit mode from 16-bit mode. The old 16-bit mode was using real mode, it used real address or physical address. But the protected 32-bit mode uses virtual address. We can also call the protected mode as user mode.

## initialize GDT0 in asmhead.asm
```
GDT0:
  RESB  8                 ; 初始值
  DW    0xffff, 0x0000, 0x9200, 0x00cf  ; 可写的32位段寄存器
  DW    0xffff, 0x0000, 0x9a28, 0x0047  ; 可执行的文件的32位寄存器

  DW    0

GDTR0:
  DW    8*3-1
  DD    GDT0
```
Note: GDT0 is a special GDT. 0 is null sector, we cannot define segment in GDT0. GDTR0 is used for letting GDT0 know there is already GDT.

Initially, GDT is in asmhead.asm rather than 0x00270000 - 0x0027ffff. IDT is not set yet and CPU is in the state that does not accept interrupts. Therefore, we need  initializa GDT/IDT, PIC and run "io_sti()" before set color palette.
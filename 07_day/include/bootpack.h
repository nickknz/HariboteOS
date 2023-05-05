#ifndef _BOOTPACK_H_
#define _BOOTPACK_H_

#define ADR_BOOTINFO 0x00000ff0

struct BootInfo{  /* 0x0ff0-0x0fff */
  char cyls; /* 启动区读硬盘读到何处为止 */
  char leds; /* 启动时键盘LED的状态 */
  char vmode; /* 显卡模式为多少位彩色 */
  char reserve;
  short scrnx, scrny; /* 画面分辨率 */
  unsigned char *vram;
};

#endif // _BOOTPACK_H_
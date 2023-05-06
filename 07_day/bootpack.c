#include <stdio.h>

#include "bootpack.h"
#include "desctbl.h"
#include "graphic.h"
#include "int.h"
#include "io.h"
#include "fifo.h"
#include "keyboard.h"

extern struct FIFO8 keyfifo;

int main(void) {
  struct BootInfo *binfo = (struct BootInfo *)ADR_BOOTINFO;
  char s[40], mcursor[256], keybuf[32];

  init_gdtidt();
  init_pic();

  io_sti(); // store interrupt flag, so that CPU can accept all interrupts from device

  init_palette();
  init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);

  fifo8_init(&keyfifo, 32, keybuf);

  int mx = (binfo->scrnx - 16) / 2;
  int my = (binfo->scrny - 28 - 16) / 2;
  init_mouse_cursor8(mcursor, COL8_008484);
  put_block8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

  sprintf(s, "(%d, %d)", mx, my);
	put_fonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

  io_out8(PIC0_IMR, 0xf9); // 开放PIC1以及键盘中断
  io_out8(PIC1_IMR, 0xef); // 开放鼠标中断

  int i;
	for (;;) {
		io_cli();
		if (fifo8_status(&keyfifo) == 0) {
			io_stihlt();
		} else {
			i = fifo8_get(&keyfifo);
			io_sti();
			sprintf(s, "%X", i);
			box_fill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
			put_fonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
		}
	}

  return 0;
}

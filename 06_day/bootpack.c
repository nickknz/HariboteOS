#include <stdio.h>

#include "bootpack.h"
#include "desctbl.h"
#include "graphic.h"
#include "int.h"
#include "io.h"

int main(void) {
  struct BootInfo *binfo = (struct BootInfo *)0x0ff0;
  char s[40], mcursor[256];

  init_gdtidt();
  init_pic();

  init_palette();
  init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
  int mx = (binfo->scrnx - 16) / 2;
  int my = (binfo->scrny - 28 - 16) / 2;
  init_mouse_cursor8(mcursor, COL8_008484);
  put_block8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

  put_fonts8_asc(binfo->vram, binfo->scrnx, 8, 8, COL8_FFFFFF, "Hello");
  put_fonts8_asc(binfo->vram, binfo->scrnx, 31, 31, COL8_000000,
                 "Haribote OS.");
  put_fonts8_asc(binfo->vram, binfo->scrnx, 30, 30, COL8_FFFFFF,
                 "Haribote OS.");

  sprintf(s, "(%d, %d)", mx, my);
	put_fonts8_asc(binfo->vram, binfo->scrnx, 0, 60, COL8_FFFFFF, s);

  for (;;) {
    io_hlt();
  }

  return 0;
}

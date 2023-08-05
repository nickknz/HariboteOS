#include <stdio.h>

#include "bootpack.h"
#include "desctbl.h"
#include "fifo.h"
#include "graphic.h"
#include "int.h"
#include "io.h"
#include "keyboard.h"
#include "memory.h"
#include "mouse.h"
#include "sheet.h"
#include "timer.h"
#include "window.h"
#include "task.h"

// new start
void task_b_main(struct Sheet *sht_back);
// new end

int main(void) {
  struct BootInfo *binfo = (struct BootInfo *)ADR_BOOTINFO;
  struct MemMan *memman = (struct MemMan *)MEMMAN_ADDR;
  struct MouseDec mdec;
  char s[40];

  unsigned int memtotal;
  struct Shtctl *shtctl;
  struct Sheet *sht_back, *sht_mouse, *sht_win, *sht_win_b[3];
  unsigned char *buf_back, buf_mouse[256], *buf_win, *buf_win_b;
  struct Timer *timer;
  struct FIFO32 fifo;
  int fifobuf[1024], data;
  struct Task *task_a, *task_b[3];

  init_gdtidt();
  init_pic(); // GDT/IDT完成初始化，开放CPU中断

  io_sti();
  fifo32_init(&fifo, 128, fifobuf, 0);
  init_pit();
  init_keyboard(&fifo, 256);
  enable_mouse(&fifo, 512, &mdec);

  io_out8(PIC0_IMR, 0xf8); // 开放PIT、PIC1以及键盘中断
  io_out8(PIC1_IMR, 0xef); // 开放鼠标中断

  memtotal = memtest(0x00400000, 0xbfffffff);
  memman_init(memman);
  // 书中为0x00001000 ~ 0x0009e000
  // 测试时发现会造成错误（原因未知）在day 16.1，所以改为由0x00010000开始
  memman_free(memman, 0x00010000, 0x0009e000); // 0x00010000 ~ 0x0009efff
  memman_free(memman, 0x00400000, memtotal - 0x00400000);

  init_palette();
  shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
  task_a = task_init(memman);
  fifo.task = task_a;
  task_run(task_a, 1, 0);

  // sht_back
  sht_back = sheet_alloc(shtctl);
  buf_back =
      (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
  sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); // 没有透明色
  init_screen8(buf_back, binfo->scrnx, binfo->scrny);

  /* sht_win_b */
  for (int i = 0; i < 3; i++) {
    sht_win_b[i] = sheet_alloc(shtctl);
    buf_win_b = (unsigned char *) memman_alloc_4k(memman, 144 * 52);
    sheet_setbuf(sht_win_b[i], buf_win_b, 144, 52, -1); /*无透明色*/
    sprintf(s, "task_b_%d", i);
    make_window8(buf_win_b, 144, 52, s, 0);
    task_b[i] = task_alloc();
    task_b[i]->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;
    task_b[i]->tss.eip = (int) &task_b_main;
    task_b[i]->tss.es = 1 * 8;
    task_b[i]->tss.cs = 2 * 8;
    task_b[i]->tss.ss = 1 * 8;
    task_b[i]->tss.ds = 1 * 8;
    task_b[i]->tss.fs = 1 * 8;
    task_b[i]->tss.gs = 1 * 8;
    *((int *) (task_b[i]->tss.esp + 4)) = (int) sht_win_b[i];
    // task_run(task_b[i], 2, i + 1);
  }
    
  // sht_win
  sht_win = sheet_alloc(shtctl);
  buf_win = (unsigned char *) memman_alloc_4k(memman, 160 * 52);
  sheet_setbuf(sht_win, buf_win, 160, 52, -1);    // 没有透明色
  make_window8(buf_win, 160, 52, "task_a", 1);
  make_textbox8(sht_win, 8, 28, 144, 16, COL8_FFFFFF);
  int cursor_x, cursor_c;
  cursor_x = 8;
  cursor_c = COL8_FFFFFF;
  timer = timer_alloc();
  timer_init(timer, &fifo, 1);
  timer_set_timer(timer, 50);

  // sht_mouse
  sht_mouse = sheet_alloc(shtctl);
  sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99); // 透明色号99
  init_mouse_cursor8(buf_mouse, 99); // 背景色号99
  int mx = (binfo->scrnx - 16) / 2; // 按在画面中央来计算坐标
  int my = (binfo->scrny - 28 - 16) / 2;

  sheet_slide(sht_back, 0, 0);
  sheet_slide(sht_win_b[0], 168, 56);
  sheet_slide(sht_win_b[1], 8, 116);
  sheet_slide(sht_win_b[2], 168, 116);
  sheet_slide(sht_win, 8, 56);
  sheet_slide(sht_mouse, mx, my);

  sheet_updown(sht_back, 0);
  sheet_updown(sht_win_b[0], 1);
  sheet_updown(sht_win_b[1], 2);
  sheet_updown(sht_win_b[2], 3);
  sheet_updown(sht_win, 4);
  sheet_updown(sht_mouse, 5);

  sprintf(s, "(%d, %d)", mx, my);
  put_fonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);  sprintf(s, "memory %dMB, free: %dKB", memtotal / (1024 * 1024),
          memman_total(memman) / 1024);
  put_fonts8_asc_sht(sht_back, 0, 32, COL8_FFFFFF, COL8_008484, s, 40);  // sheet_refresh(sht_back, 0, 0, binfo->scrnx, 48);

  for (;;) {
    io_cli(); // 只是屏蔽中断，但还是会有中断发生
    if (fifo32_status(&fifo) == 0) {
      task_sleep(task_a);
      io_stihlt(); //允许接受中断
    } else {
      data = fifo32_get(&fifo);
      io_sti();
      if (256 <= data && data <= 511) { /* 键盘数据*/
        sprintf(s, "%X", data - 256);
        put_fonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);
        if (data < 0x54 + 256 ) {
          if (keytable[data - 256] != 0 && cursor_x < 144) { /* 一般字符 */
            /* 显示1个字符就前移1次光标 */
            s[0] = keytable[data - 256];
            s[1] = 0;
            put_fonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, 1); 
            cursor_x += 8;
          }
        }
        if (data==256+0x0e && cursor_x>8) {/*退格键 */ 
          /* 用空格键把光标消去后，后移1次光标 */
          put_fonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
          cursor_x -= 8;
        }
        /* 光标再显示 */
        box_fill8(sht_win->buf,sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
        sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
      } else if (512 <= data && data <= 767) { /* 鼠标数据*/
        if (mouse_decode(&mdec, data - 512) != 0) {
          /* 已经收集了3字节的数据，所以显示出来 */
          sprintf(s, "[lcr %d %d]", mdec.x, mdec.y);

          if ((mdec.btn & 0x01)) {
            s[1] = 'L';
          }

          if ((mdec.btn & 0x02)) {
            s[3] = 'R';
          }

          if ((mdec.btn & 0x04)) {
            s[2] = 'C';
          }

          put_fonts8_asc_sht(sht_back, 32, 16, COL8_FFFFFF, COL8_008484, s, 15);

          // 移动光标
          mx += mdec.x;
          my += mdec.y;

          if (mx < 0) {
            mx = 0;
          }
          if (my < 0) {
            my = 0;
          }
          if (mx > binfo->scrnx - 1) {
            mx = binfo->scrnx - 1;
          }
          if (my > binfo->scrny - 1) {
            my = binfo->scrny - 1;
          }

          sprintf(s, "(%d, %d)", mx, my);
          put_fonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
          sheet_slide(sht_mouse, mx, my);

          sheet_slide(sht_mouse, mx, my);
          if ((mdec.btn & 0x01) != 0) {
            /* 按下左键、移动sht_win */ 
            sheet_slide(sht_win, mx - 80, my - 8);
          }
        }
      } else if (data <= 1) { /* 光标用定时器*/
        if (data == 1) {
          timer_init(timer, &fifo, 0);
          cursor_c = COL8_000000;
        } else {
          timer_init(timer, &fifo, 1);
          cursor_c = COL8_FFFFFF;
        }
        timer_set_timer(timer, 50);
        box_fill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
        sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
      } 
    }
  }

  return 0;
}
// new start


void task_b_main(struct Sheet *sht_win_b)
{
  struct FIFO32 fifo;
  struct Timer *timer_1s;
  int i, fifobuf[128], count = 0, count0 = 0;
  char s[12];

  fifo32_init(&fifo, 128, fifobuf, 0);
  timer_1s = timer_alloc();
  timer_init(timer_1s, &fifo, 100);
  timer_set_timer(timer_1s, 100);

  for (;;) {
    count++;
    io_cli();
    if (fifo32_status(&fifo) == 0) {
        io_stihlt();
    } else {
      i = fifo32_get(&fifo); 
      io_sti();
      if (i == 100) {
        sprintf(s, "%d", count - count0);
        put_fonts8_asc_sht(sht_win_b, 24, 28, COL8_000000, COL8_C6C6C6, s, 11);
        count0 = count;
        timer_set_timer(timer_1s, 100);
      }
    }
  }
}

// new end

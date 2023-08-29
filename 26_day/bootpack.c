#include <stdio.h>
#include <string.h>

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
#include "fs.h"
#include "console.h"
#include "fs.h"
#include "console.h"
#include "app.h"

int main(void) {
  struct BootInfo *binfo = (struct BootInfo *)ADR_BOOTINFO;
  struct MemMan *memman = (struct MemMan *)MEMMAN_ADDR;
  struct MouseDec mdec;
  char s[40];

  unsigned int memtotal;
  struct Shtctl *shtctl;
  struct Sheet *sht_back, *sht_mouse, *sht_cons[2];
  struct Sheet *sht = NULL, *key_win;
  unsigned char *buf_back, buf_mouse[256], *buf_cons[2];
  struct FIFO32 fifo, keycmd;
  int fifobuf[128], data, keycmd_buf[32], *cons_fifo[2];
  struct Task *task_a, *task_cons[2];
  int key_shift = 0, key_leds = (binfo->leds >> 4) & 7, keycmd_wait = -1;
  struct Console *cons;
  int x, y, mmx = -1, mmy = -1, mmx2 = 0, new_mx = -1, new_my = 0, new_wx = 0x7fffffff, new_wy = 0;

  init_gdtidt();
  init_pic(); // GDT/IDT完成初始化，开放CPU中断

  io_sti();
  fifo32_init(&fifo, 128, fifobuf, NULL);
  fifo32_init(&keycmd, 32, keycmd_buf, NULL);

  init_pit();
  init_keyboard(&fifo, 256);
  enable_mouse(&fifo, 512, &mdec);

  io_out8(PIC0_IMR, 0xf8); // 开放PIT、PIC1以及键盘中断
  io_out8(PIC1_IMR, 0xef); // 开放鼠标中断

  memtotal = memtest(0x00400000, 0xbfffffff);
  memman_init(memman);
  // 书中为0x00001000 ~ 0x0009e000
  // Note1: 测试时发现会造成错误（原因未知）在day 16.1，所以改为由0x00010000开始
  // Note2: 在640*480模式下 (day 25)，free该段内存后会导致屏幕画面错误
  // memman_free(memman, 0x00010000, 0x0009e000); // 0x00010000 ~ 0x0009efff
  memman_free(memman, 0x00400000, memtotal - 0x00400000);

  init_palette();
  shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
  task_a = task_init(memman);
  fifo.task = task_a;
  task_run(task_a, 1, 0);
  *((int *) 0x0fe4) = (int)shtctl;

  // sht_back
  sht_back = sheet_alloc(shtctl);
  buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
  sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); // 没有透明色
  init_screen8(buf_back, binfo->scrnx, binfo->scrny);

  // sht_cons
  // for (int i = 0; i < 2; i++) {
  //   sht_cons[i] = sheet_alloc(shtctl);
  //   buf_cons[i] = (unsigned char *)memman_alloc_4k(memman, 256 * 165);
  //   sheet_setbuf(sht_cons[i], buf_cons[i], 256, 165, -1); // 无透明色
  //   sprintf(s, "console%d", i);
  //   make_window8(buf_cons[i], 256, 165, s, 0);
  //   make_textbox8(sht_cons[i], 8, 28, 240, 128, COL8_000000);
  //   task_cons[i] = task_alloc();
  //   task_cons[i]->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
  //   task_cons[i]->tss.eip = (int)&console_task;
  //   task_cons[i]->tss.es = 1 * 8;
  //   task_cons[i]->tss.cs = 2 * 8;
  //   task_cons[i]->tss.ss = 1 * 8;
  //   task_cons[i]->tss.ds = 1 * 8;
  //   task_cons[i]->tss.fs = 1 * 8;
  //   task_cons[i]->tss.gs = 1 * 8;
  //   *((int *)(task_cons[i]->tss.esp + 4)) = (int)sht_cons[i];
  //   *((int *)(task_cons[i]->tss.esp + 8)) = memtotal;
  //   task_run(task_cons[i], 2, 2);   /* level=2, priority=2 */
  //   sht_cons[i]->task = task_cons[i];
  //   sht_cons[i]->flags |= 0x20;     /*有光标*/
  //   cons_fifo[i] = (int *) memman_alloc_4k(memman, 128 * 4);
  //   fifo32_init(&task_cons[i]->fifo, 128, cons_fifo[i], task_cons[i]);
  // }

  sht_cons[0] = open_console(shtctl, memtotal);
	sht_cons[1] = 0; /*未打开状态*/

  // sht_mouse
  sht_mouse = sheet_alloc(shtctl);
  sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99); // 透明色号99
  init_mouse_cursor8(buf_mouse, 99); // 背景色号99
  int mx = (binfo->scrnx - 16) / 2; // 按在画面中央来计算坐标
  int my = (binfo->scrny - 28 - 16) / 2;

  sheet_slide(sht_back, 0, 0);
  // sheet_slide(sht_cons[1], 56, 6);
  sheet_slide(sht_cons[0], 32, 4);
  sheet_slide(sht_mouse, mx, my);
  sheet_updown(sht_back, 0);
  // sheet_updown(sht_cons[1], 1);
  sheet_updown(sht_cons[0], 1);
  sheet_updown(sht_mouse, 2);

  key_win = sht_cons[0];    
  keywin_on(key_win);

  /*为了避免和键盘当前状态冲突，在一开始先进行设置*/ 
  fifo32_put(&keycmd, KEYCMD_LED); 
  fifo32_put(&keycmd, key_leds);

  for (;;) {
    if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0) {
      /*如果存在向键盘控制器发送的数据，则发送它 */ 
      keycmd_wait = fifo32_get(&keycmd); 
      wait_KBC_sendready(); 
      io_out8(PORT_KEYDAT, keycmd_wait);
    }
    io_cli(); // 只是屏蔽中断，但还是会有中断发生
    if (fifo32_status(&fifo) == 0) {
      /* FIFO为空，当存在搁置的绘图操作时立即执行*/
      if (new_mx >= 0) {
        io_sti();
        sheet_slide(sht_mouse, new_mx, new_my);
        new_mx = -1;
      } else if (new_wx != 0x7fffffff) {
        io_sti();
        sheet_slide(sht, new_wx, new_wy);
        new_wx = 0x7fffffff;
      } else {
        task_sleep(task_a);
        io_sti(); //允许接受中断
      }
    } else {
      data = fifo32_get(&fifo);
      io_sti();
      if (key_win->flags == 0) {        /*输入窗口被关闭*/
        key_win = shtctl->sheets[shtctl->top - 1];
        keywin_on(key_win);
      }
      if (256 <= data && data <= 511) { /*键盘数据*/
        sprintf(s, "%X", data - 256);
        put_fonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);

        if (data == 256 + 0x1d && key_shift != 0) {   /* Shift+control */
          struct Task *task = key_win->task;
          if (task && task->tss.ss0 != 0) {
            cons_putstr(task->cons, "\nBreak(key):\n");
            io_cli(); /*强制结束处理时禁止任务切换*/
            task->tss.eax = (int) &(task->tss.esp0);
            task->tss.eip = (int) asm_end_app;
            io_sti();
          }
        }

        if (data == 256 + 0x38 && key_shift != 0 && sht_cons[1] == 0) {   /* Shift+option */
          sht_cons[1] = open_console(shtctl, memtotal);
					sheet_slide(sht_cons[1], 32, 4);
					sheet_updown(sht_cons[1], shtctl->top);
					/*自动将输入焦点切换到新打开的命令行窗口*/
					keywin_off(key_win);
					key_win = sht_cons[1];
					keywin_on(key_win);
        }

        if (data < 0x80 + 256) {  /*将按键编码转换为字符编码*/
          if (key_shift == 0) {
            s[0] = keytable0[data - 256];
          } else {
            s[0] = keytable1[data - 256];
          }
        } else {
          s[0] = 0;
        }

        if ('A' <= s[0] && s[0] <= 'Z') {   /*当输入字符为英文字母时*/
          if (((key_leds & 4) == 0 && key_shift == 0) ||
              ((key_leds & 4) != 0 && key_shift != 0)) {
            s[0] += 0x20; /*将大写字母转换为小写字母*/
          }
        }

        if (s[0] != 0) { /*一般字符、退格键、回车键*/
          fifo32_put(&key_win->task->fifo, s[0] + 256);
        }

        if (data == 256 + 0x0e) {/*退格键*/ 
          fifo32_put(&key_win->task->fifo, 8 + 256);
        }

        if (data == 256 + 0x1c) {  // 回车键
          fifo32_put(&key_win->task->fifo, 10 + 256);
        }

        if (data == 256 + 0x0f) { /*Tab键*/
          keywin_off(key_win);
          int j = key_win->height - 1;
          if (j == 0) {
            j = shtctl->top - 1;
          }
          key_win = shtctl->sheets[j];
          keywin_on(key_win);
				}

        // 左Shift按下
        if (data == 256 + 0x2a) {
          key_shift |= 1;
        }
        // 右Shift按下
        if (data == 256 + 0x36) {
          key_shift |= 2;
        }
        // 左Shift释放
        if (data == 256 + 0xaa) {
          key_shift &= ~1;
        }
        // 右Shift释放
        if (data == 256 + 0xb6) {
          key_shift &= ~2;
        }

        // CapsLock
        if (data == 256 + 0x3a) {
          key_leds ^= 4;
          fifo32_put(&keycmd, KEYCMD_LED);
          fifo32_put(&keycmd, key_leds);
        }
        // NumLock
        if (data == 256 + 0x45) {
          key_leds ^= 2;
          fifo32_put(&keycmd, KEYCMD_LED);
          fifo32_put(&keycmd, key_leds);
        }
        // ScrollLock
        if (data == 256 + 0x46) {
          key_leds ^= 1;
          fifo32_put(&keycmd, KEYCMD_LED);
          fifo32_put(&keycmd, key_leds);
        }

        // 键盘成功接收到数据
        if (data == 256 + 0xfa) {
          keycmd_wait = -1;
        }
        // 键盘没有成功接收到数据
        if (data == 256 + 0xfe) {
          wait_KBC_sendready();
          io_out8(PORT_KEYDAT, keycmd_wait);
        }
      } else if (512 <= data && data <= 767) { /* 鼠标数据*/
        if (mouse_decode(&mdec, data - 512) != 0) {
          /* 每次鼠标移动都会收集3字节的数据 */

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

          new_mx = mx;
          new_my = my;

          if ((mdec.btn & 0x01) != 0) {
            /* 按下左键 */ 
            if (mmx < 0) {
              /*如果处于通常模式*/
              /*按照从上到下的顺序寻找鼠标所指向的图层*/
              for (int j = shtctl->top - 1; j > 0; --j) {
                sht = shtctl->sheets[j];
                x = mx - sht->vx0;
                y = my - sht->vy0;
                if (0 <= x && x < sht->bxsize && 0 <= y && y < sht->bxsize) {
                  if (sht->buf[y * sht->bxsize + x] != sht->col_inv) {
                    sheet_updown(sht, shtctl->top - 1);
                    if (sht != key_win) {
                      keywin_off(key_win);
                      key_win = sht;
                      keywin_on(key_win);
                    }
                    if (3 <= x && x < sht->bxsize - 3 && 3 <= y && y < 21) {
                      mmx = mx;   /*进入窗口移动模式*/
                      mmy = my;
                      mmx2 = sht->vx0;
                      new_wy = sht->vy0;
                    }
                    if (sht->bxsize - 21 <= x && x < sht->bxsize - 5 &&
                        5 <= y && y < 19) {
                      /*点击“×”按钮*/
                      if ((sht->flags & 0x10) != 0) {  /*该窗口是否为应用程序窗口?*/
                        struct Task *task = sht->task;
                        cons_putstr(task->cons, "\nBreak(mouse) :\n");
                        io_cli(); /*强制结束处理中禁止切换任务*/
                        task->tss.eax = (int) &(task->tss.esp0);
                        task->tss.eip = (int) asm_end_app;
                        io_sti();
                      }
                    }
                    break;
                  }
                }
              }
            } else {
              /*如果处于窗口移动模式*/
              x = mx - mmx;   /*计算鼠标的移动距离*/
              y = my - mmy;      
              new_wx = (mmx2 + x + 2) & ~3;
							new_wy = new_wy + y;
              mmy = my;       /*更新为移动后的坐标*/
            }
          } else {
            /*没有按下左键*/
            mmx = -1;   /*返回通常模式*/
            if (new_wx != 0x7fffffff) {
							sheet_slide(sht, new_wx, new_wy);	/* 固定图层位置 */
							new_wx = 0x7fffffff;
						}
          }
        }
      }
    }
  }

  return 0;
}

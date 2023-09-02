#include <stdio.h>

#include "api.h"
#include "console.h"
#include "graphic.h"
#include "sheet.h"
#include "task.h"
#include "window.h"
#include "io.h"

int *hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax) {
  struct Task *task = task_now();
  struct FIFO32 *sys_fifo = (struct FIFO32 *)*((int *) 0x0fec);
  int ds_base = task->ds_base;   // code segement address
  struct Console *cons = task->cons;
  struct Shtctl *shtctl = (struct Shtctl *)*((int *) 0x0fe4);
  struct Sheet *sht;
  char s[12];
  int *reg = &eax + 1;    /* eax后面的地址*/
  int data;
  /*强行改写通过PUSHAD保存的值*/
  /* reg[0]: EDI, reg[1]: ESI, reg[2]: EBP, reg[3]: ESP */
  /* reg[4]: EBX, reg[5]: EDX, reg[6]: ECX, reg[7]: EAX */

  switch (edx) {
    case 1:
      cons_putchar(cons, eax & 0xff, 1);
      break;
    case 2:
      cons_putstr(cons, (char *)ebx + ds_base);
      break;
    case 3:
      cons_putnstr(cons, (char *)ebx + ds_base, ecx);
      break;
    case 4:
      return &(task->tss.esp0);
      break;
    case 5:
      // EBX = window buf, ESI = x, EDI = y, EAX = sheet color, ECX = window name
      sht = sheet_alloc(shtctl);
      sht->task = task;
      sht->flags |= 0x10;
      sheet_setbuf(sht, (unsigned char *)(ebx + ds_base), esi, edi, eax);
      make_window8((unsigned char *)(ebx + ds_base), esi, edi,
                  (char *)(ecx + ds_base), 0);
      sheet_slide(sht, ((shtctl->xsize - esi) / 2) & ~3, (shtctl->ysize - edi) / 2);
      sheet_updown(sht, shtctl->top); /*将窗口图层高度指定为当前鼠标所在图层的高度，鼠标移到上层*/
      reg[7] = (int)sht;
      break;
    case 6:
      sht = (struct Sheet *)(ebx & 0xfffffffe);
      put_fonts8_asc(sht->buf, sht->bxsize, esi, edi, eax,
                    (char *)(ebp + ds_base));
      if (!(ebx & 1)) {
        sheet_refresh(sht, esi, edi, esi + ecx * 8, edi + 16);
      }
      break;
    case 7:
      sht = (struct Sheet *)(ebx & 0xfffffffe);
      box_fill8(sht->buf, sht->bxsize, ebp, eax, ecx, esi, edi);
      if (!(ebx & 1)) {
        sheet_refresh(sht, eax, ecx, esi + 1, edi + 1);
      }
      break;
    case 8:
      memman_init((struct MemMan *)(ebx + ds_base));
      ecx &= 0xfffffff0;                /*以16字节为单位*/
      memman_free((struct MemMan *)(ebx + ds_base), eax, ecx);
      break;
    case 9:
      ecx = (ecx + 0x0f) & 0xfffffff0;  /*以16字节为单位进位取整*/
      reg[7] = memman_alloc((struct MemMan *)(ebx + ds_base), ecx);
      break;
    case 10:
      ecx = (ecx + 0x0f) & 0xfffffff0;  /*以16字节为单位进位取整*/
      memman_free((struct MemMan *)(ebx + ds_base), eax, ecx);
      break;
    case 11:
      sht = (struct Sheet *)(ebx & 0xfffffffe);
      sht->buf[sht->bxsize * edi + esi] = eax;
      if ((ebx & 1) == 0) {
        sheet_refresh(sht, esi, edi, esi + 1, edi + 1);
      }
      break;
    case 12:
      sht = (struct Sheet *)ebx;
      sheet_refresh(sht, eax, ecx, esi, edi);
      break;
    case 13:
      sht = (struct Sheet *)(ebx & 0xfffffffe);
      api_line_win(sht, eax, ecx, esi, edi, ebp);
      if (!(ebx & 1)) {
        sheet_refresh(sht, esi, edi, esi + 1, edi + 1);
      }
      break;
    case 14:
      sheet_free((struct Sheet *) ebx);
      break;
    case 15:
      for (;;) {
        io_cli();

        if (!fifo32_status(&task->fifo)) {
          if (eax != 0) {
            task_sleep(task);   /* FIFO为空，休眠并等待 */
          } else {
            io_sti();
            reg[7] = -1;
            return 0;
          }
        }

        int data = fifo32_get(&task->fifo);
        io_sti();
        if (data <= 1) {  /*光标用定时器*/
          /*应用程序运行时不需要显示光标，因此总是将下次显示用的值置为1*/
          timer_init(cons->timer, &task->fifo, 1);    /*下次置为1*/
          timer_set_timer(cons->timer, 50);
        }
        if (data == 2) {  /*光标ON */
          cons->cur_c = COL8_FFFFFF;
        }
        if (data == 3) {  /*光标OFF */
          cons->cur_c = -1;
        }
        if (data == 4) {  /*只关闭命令行窗口*/
          timer_cancel(cons->timer);
          io_cli();
          fifo32_put(sys_fifo, cons->sheet - shtctl->sheets0 + 2024); /*2024~2279*/
          cons->sheet = NULL;
          io_sti();
        }
        if (256 <= data) { /*键盘数据(通过任务A)*/
          reg[7] = data - 256;
          return 0;
        }
      }

      break;
    case 16:
      reg[7] = (int) timer_alloc();
      ((struct Timer *)reg[7])->flags2 = 1;
      break;
    case 17:
      timer_init((struct Timer *) ebx, &task->fifo, eax + 256);
      break;
    case 18:
      timer_set_timer((struct Timer *) ebx, eax);
      break;
    case 19:
      timer_free((struct Timer *) ebx);
      break;
    case 20:
      if (!eax) {
        data = io_in8(0x61);
        io_out8(0x61, data & 0x0d);
      } else {
        data = 1193180000 / eax;

        io_out8(0x43, 0xb6);
        io_out8(0x42, data & 0xff);
        io_out8(0x42, data >> 8);
        
        data = io_in8(0x61);
        io_out8(0x61, (data | 0x03) & 0x0f);
      }
      break;
    default:
      break;
  }

  return 0;
}

void api_line_win(struct Sheet *sht, int x0, int y0, int x1, int y1, int col) {
  int dx = x1 - x0;
  int dy = y1 - y0;
  int x = x0 << 10;
  int y = y0 << 10;
  int len = 0;

  if (dx < 0) {
    dx = -dx;
  }
  if (dy < 0) {
    dy = -dy;
  }

  if (dx >= dy) {
    len = dx + 1;   // 当起点和终点完全相 同时，应该在画面上画出1个点
    if (x0 > x1) {
      dx = -1024;
    } else {
      dx = 1024;
    }

    if (y0 <= y1) {
      dy = ((y1 - y0 + 1) << 10) / len;
    } else {
      dy = ((y1 - y0 - 1) << 10) / len;
    }
  } else {
    len = dy + 1;
    if (y0 > y1) {
      dy = -1024;
    } else {
      dy = 1024;
    }

    if (x0 <= x1) {
      dx = ((x1 - x0 + 1) << 10) / len;
    } else {
      dx = ((x1 - x0 - 1) << 10) / len;
    }
  }

  for (int i = 0; i < len; i++) {
    sht->buf[(y >> 10) * sht->bxsize + (x >> 10)] = col;
    x += dx;
    y += dy;
  }
}
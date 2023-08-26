#include <stdio.h>
#include <string.h>

#include "bootpack.h"
#include "console.h"
#include "desctbl.h"
#include "fifo.h"
#include "fs.h"
#include "graphic.h"
#include "memory.h"
#include "mouse.h"
#include "io.h"
#include "sheet.h"
#include "task.h"
#include "timer.h"
#include "command.h"


void console_task(struct Sheet *sheet, unsigned int memtotal)
{
	struct Timer *timer;
	struct Task *task = task_now();
	struct MemMan *memman = (struct MemMan *) MEMMAN_ADDR;
	int i, fifobuf[128], *fat = (int *)memman_alloc_4k(memman, 4 * 2880);
	struct Console cons;
	char s[30], cmdline[30];
	cons.sheet = sheet;
	cons.cur_x = 8;
	cons.cur_y = 28;
	cons.cur_c = -1;

	struct SegmentDescriptor *gdt = (struct SegmentDescriptor *) ADR_GDT;
	int x, y;

	// *((int *)0x0fec) = (int)&cons;
	task->cons = &cons;

	fifo32_init(&task->fifo, 128, fifobuf, task);

	cons.timer = timer_alloc();
	timer_init(cons.timer, &task->fifo, 1);
	timer_set_timer(cons.timer, 50);

	file_read_fat(fat, (unsigned char *)(ADR_DISKIMG + 0x000200));

	/*显示提示符prompt*/
	put_fonts8_asc_sht(sheet, 8, 28, COL8_FFFFFF, COL8_000000, ">", 1);  
	cons_putchar(&cons, '>', 1);

	for (;;) {
		io_cli();
		if (fifo32_status(&task->fifo) == 0) {
			task_sleep(task);
			io_sti();
		} else {
			i = fifo32_get(&task->fifo);
			io_sti();
			if (i <= 1) { /* 光标用定时器 */
				if (i != 0) {
					timer_init(cons.timer, &task->fifo, 0); /* 下次置0 */
					if (cons.cur_c >= 0) {
						cons.cur_c = COL8_FFFFFF;
					}
				} else {
					timer_init(cons.timer, &task->fifo, 1); /* 下次置1 */
					if (cons.cur_c >= 0) {
						cons.cur_c = COL8_000000;
					}
				}
				timer_set_timer(cons.timer, 50);
			} else if (i == 2) {  // 窗口光标ON
				cons.cur_c = COL8_FFFFFF;
			} else if (i == 3) {  // 窗口光标OFF
				box_fill8(sheet->buf, sheet->bxsize, COL8_000000, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
				cons.cur_c = -1;
			} else if (256 <= i && i <= 511) { /* 键盘数据(通过任务A) */ 
				if (i == 8 + 256) {   /* 退格键 */
					if (cons.cur_x > 16) {
						/* 用空白擦除光标后将光标前移一位 */
						cons_putchar(&cons, ' ', 0);
						cons.cur_x -= 8;
					}
				} else if (i == 10 + 256) {   // 回车键 Enter
					/* 用空格将光标擦除 */
					cons_putchar(&cons, ' ', 0);
					int len = cons.cur_x / 8 - 2;
					cmdline[cons.cur_x / 8 - 2] = '\0';
					char *trimed_cmdline = trim(cmdline, len);
					cons_newline(&cons);
					cons_run_cmd(trimed_cmdline, &cons, fat, memtotal);
					/* 显示提示符 */
					cons_putchar(&cons, '>', 1);
				} else {
					/* 一般字符 */
					if (cons.cur_x < 240) {
						/* 显示一个字符之后将光标后移一位 */
						cmdline[cons.cur_x / 8 - 2] = i - 256;
						cons_putchar(&cons, i - 256, 1);
					}
				}
			}
			/* 重新显示光标 */
			if (cons.cur_c >= 0) {
				box_fill8(sheet->buf, sheet->bxsize, cons.cur_c, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
			}
			sheet_refresh(sheet, cons.cur_x, cons.cur_y, cons.cur_x + 8, cons.cur_y + 16);
		}
	}
}

void cons_putchar(struct Console *cons, int ch, char move) 
{
	char s[2];

	s[0] = ch;
	s[1] = '\0';

	if (s[0] == '\t') {				/*制表符 tab*/
		for (;;) {
			put_fonts8_asc_sht(cons->sheet, cons->cur_x, cons->cur_y, COL8_FFFFFF,
								COL8_000000, " ", 1);
			cons->cur_x += 8;
			if (cons->cur_x == 8 + 240) {
				cons_newline(cons);
			}

			if (!((cons->cur_x - 8) & 0x1f)) {
				break; // 被32整除则break
			}
		}
	} else if (s[0] == '\n') {		/*换行*/
		// 换行符
		cons_newline(cons);
	} else if (s[0] == '\r') {		/*换行*/
		// 回车符，暂不处理
	} else {						/*一般字符*/
		put_fonts8_asc_sht(cons->sheet, cons->cur_x, cons->cur_y, COL8_FFFFFF,
						COL8_000000, s, 1);
		if (move) {
			/* move为0时光标不后移*/
			cons->cur_x += 8;
			if (cons->cur_x == 8 + 240) {
				cons_newline(cons);
			}
		}
	}
}

void cons_newline(struct Console *cons)
{
	struct Sheet *sheet = cons->sheet;

	if (cons->cur_y < 28 + 112) {
		cons->cur_y += 16;	/*到下一行*/
	} else {
		// 滚动
		for (int y = 28; y < 28 + 112; y++) {
			for (int x = 8; x < 8 + 240; x++) {
				sheet->buf[x + y * sheet->bxsize] =
					sheet->buf[x + (y + 16) * sheet->bxsize];
			}
		}
		for (int y = 28 + 112; y < 28 + 128; y++) {
			for (int x = 8; x < 8 + 240; x++) {
				sheet->buf[x + y * sheet->bxsize] = COL8_000000;
			}
		}

		sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
	}

	cons->cur_x = 8;
}

void cons_putstr(struct Console *cons, char *s) 
{
	while (*s) {
		cons_putchar(cons, *s, 1);
		s++;
	}
}

void cons_putnstr(struct Console *cons, char *s, int n) 
{
	for (int i = 0; i < n; i++) {
		cons_putchar(cons, s[i], 1);
	}
}

void cons_run_cmd(char *cmdline, struct Console *cons, int *fat,
                  unsigned int memtotal) 
{
	if (!strcmp(cmdline, "mem")) {
		cmd_mem(cons, memtotal);
	} else if (!strcmp(cmdline, "clear")) {
		cmd_clear(cons);
	} else if (!strcmp(cmdline, "ls")) {
		cmd_ls(cons);
	} else if (!strncmp(cmdline, "cat ", 4)) {
		cmd_cat(cons, fat, cmdline);
	} else if (cmdline[0] != '\0') {
		if (!cmd_app(cons, fat, cmdline)) {
			/*不是命令，不是应用程序，也不是空行*/
			cons_putstr(cons, "Not found command.\n\n");
		}
	}
}


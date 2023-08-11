#include <stdio.h>
#include <string.h>

#include "console.h"
#include "graphic.h"
#include "sheet.h"
#include "io.h"
#include "fifo.h"
#include "task.h"
#include "fs.h"

void console_task(struct Sheet *sheet, unsigned int memtotal)
{
	struct Task *task = task_now();
	int i, fifobuf[128], cursor_x = 16, cursor_y = 28, cursor_c = -1;
	struct Timer *timer;
	char s[30], cmdline[30];
	struct MemMan *memman = (struct MemMan *) MEMMAN_ADDR;
	struct FileInfo *finfo = (struct FileInfo *) (ADR_DISKIMG + 0x002600);
	int *fat = (int *)memman_alloc_4k(memman, 4 * 2880);

	fifo32_init(&task->fifo, 128, fifobuf, task);

	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_set_timer(timer, 50);

	file_read_fat(fat, (unsigned char *)(ADR_DISKIMG + 0x000200));

	/*显示提示符prompt*/
	put_fonts8_asc_sht(sheet, 8, 28, COL8_FFFFFF, COL8_000000, ">", 1);  

	for (;;) {
		io_cli();
		if (fifo32_status(&task->fifo) == 0) {
			task_sleep(task);
			io_sti();
		} else {
			i = fifo32_get(&task->fifo);
			io_sti();
			if (i <= 1) { /*光标用定时器*/
				if (i != 0) {
					timer_init(timer, &task->fifo, 0); /*下次置0 */
					if (cursor_c >= 0) {
						cursor_c = COL8_FFFFFF;
					}
				} else {
					timer_init(timer, &task->fifo, 1); /*下次置1 */
					if (cursor_c >= 0) {
						cursor_c = COL8_000000;
					}
				}
				timer_set_timer(timer, 50);
			} else if (i == 2) {  // 窗口光标ON
				cursor_c = COL8_FFFFFF;
			} else if (i == 3) {  // 窗口光标OFF
				box_fill8(sheet->buf, sheet->bxsize, COL8_000000, cursor_x, 28, cursor_x + 7, 43);
				cursor_c = -1;
			} else if (256 <= i && i <= 511) { /*键盘数据(通过任务A) */ 
				if (i == 8 + 256) {   /*退格键*/
					if (cursor_x > 16) {
						/*用空白擦除光标后将光标前移一位*/
						put_fonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1); 
						cursor_x -= 8;
					}
				} else if (i == 10 + 256) {   // 回车键 Enter
					/*用空格将光标擦除*/
					put_fonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
					int len = cursor_x / 8 - 2;
					cmdline[cursor_x / 8 - 2] = 0;
					cursor_y = cons_newline(cursor_y, sheet);
					/*执行命令*/
					char *trimed_cmdline = trim(cmdline, len);
					if (strcmp(trimed_cmdline, "mem") == 0) {
						/* mem command */
						sprintf(s, "total   %dMB", memtotal / (1024 * 1024));
						put_fonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
						cursor_y = cons_newline(cursor_y, sheet);
						sprintf(s, "free %dKB", memman_total(memman) / 1024);
						put_fonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
						cursor_y = cons_newline(cursor_y, sheet);
						cursor_y = cons_newline(cursor_y, sheet);
					} else if (strcmp(trimed_cmdline, "clear") == 0) {   
						/* clear command */
						for (int y = 28; y < 28 + 128; y++) {
							for (int x = 8; x < 8 + 240; x++) {
								sheet->buf[x + y * sheet->bxsize] = COL8_000000;
							}
						}
						sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
						cursor_y = 28;
					} else if (strcmp(trimed_cmdline, "ls") == 0) {
						/* ls command */
						for (int x = 0; x < 224; x++) {
							if (finfo[x].name[0] == '\0') {
								break;
							}

							if (finfo[x].name[0] != 0xe5) {
								if ((finfo[x].type & 0x18) == 0) {
									sprintf(s, "filename.ext   %d", finfo[x].size);
									for (int y = 0; y < 8; y++) {
										s[y] = finfo[x].name[y];
									}
									s[9] = finfo[x].ext[0];
									s[10] = finfo[x].ext[1];
									s[11] = finfo[x].ext[2];
									put_fonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
									cursor_y = cons_newline(cursor_y, sheet);
								}
							}
						}
						cursor_y = cons_newline(cursor_y, sheet);
					} else if (strncmp(trimed_cmdline, "cat ", 4) == 0) {
						/* cat command */
						int x, y;
						/*准备文件名*/
						for (y = 0; y < 11; y++) {
							s[y] = ' ';
						}
						y = 0;

						for (x = 4; y < 11 && cmdline[x] != '\0'; x++) {
							if (cmdline[x] == '.' && s[y] <= 'z') {
								y = 8;
							} else {
								s[y] = cmdline[x];
								if ('a' <= s[y] && s[y] <= 'z') {
									// 转为大写
									s[y] -= 0x20;
								}
								y++;
							}
						}
						// 寻找文件
						for (x = 0; x < 224;) {
							if (finfo[x].name[0] == '\0') {
								break;
							}
							if (!(finfo[x].type & 0x18)) {
								for (y = 0; y < 11; y++) {
									if (finfo[x].name[y] != s[y]) {
										goto type_next_file;
									}
								}
								break;  /*找到文件*/
							}

							type_next_file:
							x++;
						}
						if (x < 224 && finfo[x].name[0] != '\0') {
							/*找到文件的情况*/
							char *p = (char *)memman_alloc_4k(memman, finfo[x].size);
							file_load_file(finfo[x].clustno, finfo[x].size, p, fat, (char *)(ADR_DISKIMG + 0x003e00));
							cursor_x = 8;
							for (y = 0; y < finfo[x].size; ++y) {
								/*逐字输出*/
								s[0] = p[y];
								s[1] = '\0';

								if (s[0] == '\t') {          /*制表符 tab缩进*/
									for (;;) {
										put_fonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
										cursor_x += 8;
										if (cursor_x == 8 + 240) {
											cursor_x = 8;
											cursor_y = cons_newline(cursor_y, sheet);
										}
										if (!((cursor_x - 8) & 0x1f)) {
											break; // 被32整除则break
										}
									}
								} else if (s[0] == '\n') {   /*换行*/
									cursor_x = 8;
									cursor_y = cons_newline(cursor_y, sheet);
								} else if (s[0] == '\r') {   /*回车*/
									// 回车符，暂不处理
								} else {                     /*一般字符*/
									put_fonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, s, 1);
									cursor_x += 8;
									if (cursor_x == 8 + 240) { /*到达最右端后换行*/ 
										cursor_x = 8;
										cursor_y = cons_newline(cursor_y, sheet);
									}
								}
							}
							memman_free_4k(memman, (int)p, finfo[x].size);
						} else {
							/*没有找到文件的情况*/
							put_fonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000,
											"File not found.", 15);
							cursor_y = cons_newline(cursor_y, sheet);
						}
						cursor_y = cons_newline(cursor_y, sheet);
					} else if (trimed_cmdline[0] != 0) {
						/*不是命令，也不是空行 */
						put_fonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, "Not found command.", 19);
						cursor_y = cons_newline(cursor_y, sheet);
						cursor_y = cons_newline(cursor_y, sheet);
					}
					/*显示提示符*/
					put_fonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, ">", 1); 
					cursor_x = 16;
				} else {
					/*一般字符*/
					if (cursor_x < 240) {
						/*显示一个字符之后将光标后移一位 */
						s[0] = i - 256;
						s[1] = 0;
						cmdline[cursor_x / 8 - 2] = i - 256;
						put_fonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, s, 1); 
						cursor_x += 8;
					} 
				}
			}
			/*重新显示光标*/
			if (cursor_c >= 0) {
				box_fill8(sheet->buf, sheet->bxsize, cursor_c, cursor_x, cursor_y, cursor_x + 7, cursor_y + 15);
			}
			sheet_refresh(sheet, cursor_x, cursor_y, cursor_x + 8, cursor_y + 16);
		}
	}
}

int cons_newline(int cursor_y, struct Sheet *sheet)
{
	int x, y;
	if (cursor_y < 28 + 112) {
		cursor_y += 16; /*换行*/
	} else {  /*滚动*/
		for (y = 28; y < 28 + 112; y++) {
			for (x = 8; x < 8 + 240; x++) {
				sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
			}
		}
		for (y = 28 + 112; y < 28 + 128; y++) {
			for (x = 8; x < 8 + 240; x++) {
				sheet->buf[x + y * sheet->bxsize] = COL8_000000;
			}
		}
		sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
	}
	return cursor_y;
}
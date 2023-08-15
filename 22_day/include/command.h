#include "console.h"

#ifndef _COMMAND_H_
#define _COMMAND_H_

void cmd_mem(struct Console *cons, unsigned int memtotal);
void cmd_clear(struct Console *cons);
void cmd_ls(struct Console *cons);
void cmd_cat(struct Console *cons, int *fat, char *cmdline);
void cmd_hlt(struct Console *cons, int *fat);
int cmd_app(struct Console *cons, int *fat, char *cmdline);

#endif // _COMMAND_H_
#include "fifo.h"
#include "io.h"
#include "keyboard.h"
#include "mouse.h"

struct FIFO8 mousefifo;
unsigned char mousebuf[MOUSE_FIFO_BUF_SIZE];

/* 激活鼠标 */
void enable_mouse(void) {
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
    return; /* 顺利的话,键盘控制其会返送回ACK(0xfa)*/
}
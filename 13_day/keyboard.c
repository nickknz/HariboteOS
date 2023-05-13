#include "fifo.h"
#include "keyboard.h"
#include "io.h"

struct FIFO8 keyfifo;
unsigned char keybuf[KEY_FIFO_BUF_SIZE];

void wait_KBC_sendready(void) {
    /* 等待键盘控制电路准备完毕 */
    for (;;) {
        if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
            break;
        }
    }
}

// init KBC (Keyboard Controller)
void init_keyboard(void) {
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, KBC_MODE);
}
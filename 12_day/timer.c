// #include <stdio.h>

#include "int.h"
#include "io.h"
#include "timer.h"

struct TIMERCTL timerctl;

void init_pit(void)
{
    io_out8(PIT_CTRL, 0x34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e);
    timerctl.count = 0; /* 这里！ */
    return;
}



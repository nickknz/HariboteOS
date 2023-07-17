## Timer
``` C
struct Timer {
  unsigned int flags;
  unsigned int timeout;
  struct FIFO8 *fifo;
  unsigned char data;
};

struct TimerCtl {
  unsigned int count, next, using;
  struct Timer *timers[500];
  struct Timer timers0[500];
};
```
Initially, we set timerctl.count as 0 in init_pit(). And we can set max 500 timers. flags is for set state for each timer (0 means not using).

``` C
void int_handler20(int *esp) {
  int i, j;

  io_out8(PIC0_OCW2, 0x60); // 接收IRQ-00信号通知PIC
  timerctl.count++;

  if (timerctl.next > timerctl.count) {
    return;
  }

  for (i = 0; i < timerctl.using; i++) {
    // timers的定时器都处于动作中，所以不确认flags
    if (timerctl.timers[i]->timeout > timerctl.count) {
      break;
    }

    // 超时
    timerctl.timers[i]->flags = TIMER_FLAGS_ALLOC;
    fifo8_put(timerctl.timers[i]->fifo, timerctl.timers[i]->data);
  }

  // 正好有i个定时器超时了，其余的进行移位
  timerctl.using -= i;
  for (j = 0; j < timerctl.using; j++) {
    timerctl.timers[j] = timerctl.timers[i + j];
  }

  if (timerctl.using > 0) {
    timerctl.next = timerctl.timers[0]->timeout;
  } else {
    timerctl.next = 0xffffffff;
  }
}
```

For each 10ms, the timer will interrupt the CPU or int_handler20 will be called, so CPU will get 100 times or count will increase 100 in each 1 second.
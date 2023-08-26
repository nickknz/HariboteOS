#include "api.h"

int main() {
  int timer = api_alloc_timer();
  api_init_timer(timer, 128);

  for (int i = 20000000; i >= 20000; i -= i / 100) {
    /* 20KHz~20Hz，即人类可以听到的声音范围*/
    /* i以1%的速度递减*/
    api_beep(i);
    api_set_timer(timer, 1);    // 0.01秒
    if (api_get_key(1) != 128) {
      break;
    }
  }

  api_beep(0);
  
  api_end();

  return 0;
}
#include "fifo.h"

/* 初始化FIFO缓冲区 */
void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf) {
  fifo->size = size;
  fifo->buf = buf;
  fifo->free = size;
  fifo->flags = 0;
  fifo->next_r = 0;
  fifo->next_w = 0;
}

/* 向FIFO传送数据并保存 */
int fifo8_put(struct FIFO8 *fifo, unsigned char data) {
  if (fifo->free == 0) {    /* 空余没有了，溢出 */
    fifo->flags |= FLAGS_OVERRUN;
    return -1;
  }

  fifo->buf[fifo->next_w++] = data;
  if (fifo->next_w == fifo->size) {
    fifo->next_w = 0;
  }
  fifo->free--;
  return 0;
}

/* 从FIFO取得一个数据 */
int fifo8_get(struct FIFO8 *fifo) {
  if (fifo->free == fifo->size) {   /* 如果缓冲区为空，则返回 -1 */
    return -1;
  }

  int data = fifo->buf[fifo->next_r++];
  if (fifo->next_r == fifo->size) {
    fifo->next_r = 0;
  }
  fifo->free++;
  return data;
}

/* 报告一下到底积攒了多少数据 */
int fifo8_status(struct FIFO8 *fifo) {
  return fifo->size - fifo->free;
}
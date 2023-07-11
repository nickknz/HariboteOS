The core concept of this chapter is reading 3 bytes from mouse moving. It's greate to understand how we decode the data receiving from mouse moving. The first byte indicates if click the mouse left or right. THe second byte indicates x and the third one shows the y.
``` C
int mouse_decode(struct MouseDec *mdec, unsigned char dat) {
  if (mdec->phase == 0) {
    // 等待鼠标的0xfa状态
    if (dat == 0xfa) {
      mdec->phase = 1;
    }
    return 0;
  } else if (mdec->phase == 1) {
    // 等待鼠标的第一个字节
    if ((dat & 0xc8) == 0x08) {
      // 如果第一个字节正确
      mdec->buf[0] = dat;
      mdec->phase = 2;
    }
    return 0;
  } else if (mdec->phase == 2) {
    // 等待鼠标的第二个字节
    mdec->buf[1] = dat;
    mdec->phase = 3;
    return 0;
  } else if (mdec->phase == 3) {
    // 等待鼠标的第三个字节
    mdec->buf[2] = dat;
    mdec->phase = 1;

    mdec->btn = mdec->buf[0] & 0x07;
    mdec->x = mdec->buf[1];
    mdec->y = mdec->buf[2];

    if ((mdec->buf[0] & 0x10) != 0) {
      mdec->x |= 0xffffff00;
    }

    if ((mdec->buf[0] & 0x20) != 0) {
      mdec->y |= 0xffffff00;
    }

    mdec->y = -mdec->y; // 鼠标的y方向与画面符号相反

    return 1;
  }
  return -1;
}
```

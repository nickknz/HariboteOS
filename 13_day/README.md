## The data write into FIFO Buffer
We refactor the code for FIFO from FIFO8 (unsigned char) to FIFO32 (int).
0~ 1.....................光标闪烁用定时器
3.....................3秒定时器
10.....................10秒定时器
256~ 511.....................键盘输入(从键盘控制器读入的值再加上256) 512~ 767......鼠标输入(从键盘控制器读入的值再加上512)


There is something about timer we need to be noted:
The next in Timer is next Timer address, but the next in TimerCtl is next timer's timeoout.
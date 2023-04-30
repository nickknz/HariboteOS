#include <stdio.h>

// void io_hlt(void);

// 入口函数HariMain重命名为标准的main
// 返回类型修改为int，避免编译器警告
int main(void) {
    fin:
    printf("Hello, World!\n");
    // io_hlt();   /*执行func.asm里的io_hlt*/
    goto fin;
}
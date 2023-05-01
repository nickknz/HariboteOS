void io_hlt(void);
void write_mem8(int addr, int data);

int main(void)
{
	int i; /*变量声明。变量i是32位整数*/
    char *p; /*变量p，用于BYTE型地址*/
    for (i = 0xa0000; i <= 0xaffff; i++) {
        p = i; /*代入地址*/
        *p = i & 0x0f;
        /*这可以替代write_mem8(i, i & 0x0f);*/
    }

    for (;;) {
        io_hlt();
    }
}
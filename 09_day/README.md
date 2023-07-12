## Memory management
We are not going to use bitmap to manage the memory. Instead, we are going to record the memory information such as the memory address between start and end is free.

There is simple version below:
``` C
struct FreeInfo {   /* 可用信息 */
  unsigned int addr, size;
};

struct MemMan {     /* 内存管理 */
  int frees;
  struct FreeInfo free[1000];
};

struct MemMan memman;
memman.frees = 1; // there is onely one free space we can use currently
memman.free[0].addr = 0x00400000; // starts from 0x00400000, there are 124MB free space
memman.free[0].size = 0x07c00000;
```
If we need 100KB space, we can find it through memman.free
``` C
for (int i = 0; i < memman.freees; ++i) {
    if (mamman.free[i].size >= 100 * 1024) {
        // Find the free space!
    }
}
```
If we find the free space we wanna use, the free space could be allocated
``` C
memman.free[i].addr += 100 * 1024;   // 可用地址向后推进100KB   
memman.free[i].size -= 100 * 1024;   // size 减去100KB   
```
If one of free space's size equal 0, then this space is not free and frees--

When we do free() the space, it is possible freee++ and check if 2 free spaces are continuous, if so, we need to link or concatenate those 2 free space as one entire free space.

Let's "make qemu"
It shows memory 128MB  free: 127608KB which is 632KB + 124 * 1024. This makes sense to me.
## Shtctl struct
``` C
struct Sheet {
  unsigned char *buf;
  int bxsize, bysize, vx0, vy0, col_inv, height, flags;
};

struct Shtctl {
  unsigned char *vram;
  int xsize, ysize, top;
  struct Sheet *sheets[MAX_SHEETS];
  struct Sheet sheets0[MAX_SHEETS];
};
``` 

![Image 2023-07-12 at 7 52 PM](https://github.com/Nick-zhen/HariboteOS/assets/62523802/1218b31f-ff54-4bec-bcbf-717c718ef506)

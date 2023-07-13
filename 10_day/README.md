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

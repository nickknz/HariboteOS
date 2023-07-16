## Create new Window
``` C
unsigned char *buf_win = (unsigned char *) memman_alloc_4k(memman, 160 * 52);
struct *sht_win = sheet_alloc(shtctl);
sheet_setbuf(sht_win, buf_win, 160, 52, -1); 
make_window8(buf_win, 160, 52, "counter");
sheet_slide(sht_win, 80, 72);

sheet_updown(sht_back, 0);
sheet_updown(sht_win, 1);
sheet_updown(sht_mouse, 2);
```

In today, we improve the refresh algo. We only refresh the sheets that are above the changed object sheet. However, when the mouse is on the counter, the mouse will keep twinkle. Then we are gonna use map algorithm.
``` C
struct Shtctl {
  unsigned char *vram, *map;
  int xsize, ysize, top;
  struct Sheet *sheets[MAX_SHEETS];
  struct Sheet sheets0[MAX_SHEETS];
};
  // inside shtctl_init
  ctl->map = (unsigned char *)memman_alloc_4k(memman, xsize * ysize);
  if (!ctl->map) {
    memman_free_4k(memman, (int)ctl, sizeof(struct Shtctl));
    return NULL;
  }
```
We create a new heap memory named map for representing which pixel that is belong to which sheet. It is like the map of sheets. Then we can directly refresh sheets from seeing the map without worrying about the problem of overlaps.
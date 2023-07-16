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

In today, we improve the refresh algo. We only refresh the sheets that are above the changed object sheet.
#define PIT_CTRL	0x0043
#define PIT_CNT0	0x0040

extern struct TIMERCTL timerctl;

struct TIMERCTL {
    unsigned int count;
};

void init_pit(void);



  ; [FORMAT "WCOFF"]
  [BITS 32]          ; 制作32位模式用的机械语言

  GLOBAL io_hlt      ; 程序中包含函数名

  [SECTION .text]
io_hlt:              ; void io_hlt(void);
  HLT
  RET
  ; [FORMAT "WCOFF"]
  [BITS 32]          ; 制作32位模式用的机械语言

  GLOBAL io_hlt, write_mem8      ; 程序中包含函数名

  [SECTION .text]
io_hlt:              ; void io_hlt(void);
  HLT
  RET

write_mem8: ; void write_mem8(int addr, int data);
  MOV ECX,[ESP+4] ; [ESP + 4]中存放的是地址，将其读入ECX
  MOV AL,[ESP+8] ; [ESP + 8]中存放的是数据，将其读入AL
  MOV [ECX],AL
  RET
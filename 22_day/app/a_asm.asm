  [BITS 32]

  GLOBAL api_putchar, api_end

api_putchar:    ; void api_puchar(int c);
  MOV   EDX, 1
  MOV   AL, [ESP+4]
  INT   0x40
  RET

api_end:
  MOV   EDX, 4
  INT   0x40
  [BITS 32]

  GLOBAL asm_hrb_api

  EXTERN hrb_api

asm_hrb_api:
  STI
  PUSHAD            ; 用于保存寄存器值的PUSH
  
  PUSHAD            ; 用于保存寄存器值的PUSH
  CALL    hrb_api
  ADD     ESP, 32
  POPAD
  IRETD
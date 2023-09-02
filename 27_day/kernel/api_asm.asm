  [BITS 32]

  GLOBAL asm_hrb_api

  EXTERN hrb_api

asm_hrb_api:
  STI
  PUSH    DS
  PUSH    ES
  PUSHAD            ; 用于保存的PUSH
  PUSHAD            ; 用于向hrb_api传值的PUSH
  MOV     AX, SS    
  MOV     DS, AX    ; 将操作系统用段地址存入DS和ES
  MOV     ES, AX
  CALL    hrb_api
  CMP     EAX, 0    ; 当EAX不为0时程序结束
  JNE     .end_app
  ADD     ESP, 32
  POPAD
  POP     ES
  POP     DS
  IRETD             ; 这个命令会自动执行STI

.end_app:
; EAX为tss.esp0的地址
  MOV     ESP, [EAX]
  POPAD
  RET               ; 返回cmd_app
    [BITS 32]

    MOV     EDX,2
    MOV     EBX,msg
    INT     0x40
    RETF

msg:
    DB "hello world!",0

;   [BITS 32]

;   MOV   EDX, 2
;   MOV   EBX, msg
;   INT   0x40
  
;   MOV   EDX, 4
;   INT   0x40

; msg:
;   DB    "hello", 0
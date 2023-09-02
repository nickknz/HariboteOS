  [BITS 32]

  GLOBAL load_tr
  GLOBAL far_jmp, far_call
  GLOBAL taskswitch4
  GLOBAL taskswitch3

; task register, 让CPU记住当前运行哪个task
load_tr:            ; void load_tr(int tr);
  LTR   [ESP+4]     ; tr
  RET

far_jmp:            ; void far_jmp(int eip, int cs);
  JMP   FAR [ESP+4]
  RET

far_call:           ; void far_call(int eip, int cs);
  CALL  FAR [ESP+4] ;; eip, cs
  RET

taskswitch4:   ; void taskswitch4(void);
  JMP     4*8:0
  RET

taskswitch3:   ; void taskswitch3(void);
  JMP     3*8:0
  RET


  [BITS 32]

  GLOBAL start_app

start_app:                ; void start_app(int eip, int cs, int esp, int ds, int *tss_esp0);
  PUSHAD                  ; 将32位寄存器的值全部保存起来
  MOV     EAX, [ESP+36]   ; EIP
  MOV     ECX, [ESP+40]   ; CS
  MOV     EDX, [ESP+44]   ; ESP
  MOV     EBX, [ESP+48]   ; DS/SS
  MOV     EBP, [ESP+52]   ; tss.esp0
  MOV     [EBP], ESP      ; 保存操作系统用ESP
  MOV     [EBP+4], SS     ; 保存操作系统用SS
  MOV     ES, BX
  MOV     DS, BX
  MOV     FS, BX
  MOV     GS, BX
; 下面调整栈，以免用RETF跳转到应用程序
  OR      ECX, 3          ; 将应用程序用段号和3进行OR运算
  OR      EBX, 3          ; 将应用程序用段号和3进行OR运算
  PUSH    EBX             ; 应用程序的SS
  PUSH    EDX             ; 应用程序的ESP
  PUSH    ECX             ; 应用程序的CS
  PUSH    EAX             ; 应用程序的EIP
  RETF
; 应用程序结束后不会回到这里
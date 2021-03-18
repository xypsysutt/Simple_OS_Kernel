BITS 16
[extern OS]
[extern INT08H_START]
[extern INT20H_START]
[extern INT21H_START]
[extern INT22H_START]
[extern RESTART]
[extern Timer]
%include "header.asm"
global _start
_start:  
    WriteIVT 08h, INT08H_START
    WriteIVT 20h, INT20H_START
    WriteIVT 21h, INT21H_START
    WriteIVT 22h, INT22H_START
    call dword OS 
    jmp _start    


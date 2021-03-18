BITS 16
[extern OS]

global _start
_start:  
    call dword OS 
    jmp _start    
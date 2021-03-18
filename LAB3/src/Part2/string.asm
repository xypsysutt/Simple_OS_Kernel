section .data
msg:
        db  'hello world'
.len    equ $ - msg

global my_str
global my_print
section .text
my_str:
    mov eax,msg
    ret
my_print:
    mov  ecx,[esp+4] 
    mov edx,4
    mov ebx,1
    mov eax,4
    int 80h
    ret

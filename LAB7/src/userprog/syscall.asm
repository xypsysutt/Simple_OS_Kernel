org 05000h
%include "../header.asm"
Start:
    mov ax,cs
    mov es,ax              ; ES = CS
    mov ds,ax              ; DS = CS
    mov es,ax              ; ES = CS
    mov ax,0B800h
    mov gs,ax  


    mov ax, 0003h
    int 10h                       

    PRINTLINE msg1,len1,0,0
    PRINTLINE msg2,len2,2,0
    int 22h
    PRINTLINE msg,len,15,20

    mov ah, 0
    int 16h
;************************************
    mov ax, 0003h
    int 10h

    PRINTLINE msg3,len3,2,0
    mov ah,0
    int 21h
    PRINTLINE msg,len,15,20

    mov ah, 0
    int 16h
;************************************
    mov ax, 0003h
    int 10h

    PRINTLINE msg4,len4,2,0
    mov ah,1
    int 21h ;reBoot
    PRINTLINE msg,len,15,20

    mov ah, 0
    int 16h
;************************************


Datedef:
    msg1 db 'This prog will excute INT22 --> INT21H'
    len1 equ ($ - msg1)
    msg2 db 'INT22H(print int22h)'
    len2 equ ($ - msg2)
    msg3 db 'INT21H AH=0(show Time)'
    len3 equ ($ - msg3)
    msg4 db 'INT21H AH=1(reBoot)'
    len4 equ ($ - msg4)
    msg5 db 'INT21H AH=2 is to PowerOff, but i have no idea to show this function'
    msg db 'press any key to continue'
    len equ ($ - msg)

    times 1024-($-$$) db 0
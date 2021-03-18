org 08100h
prog1 equ 08300h
prog2 equ 08700h
prog3 equ 08B00h
prog4 equ 08F00h

start:
    mov ax,cs
	mov es,ax					; ES = 0
	mov ds,ax					; DS = CS
	mov es,ax					; ES = CS
	mov ax,0B800h				; �ı������Դ���ʼ��ַ
	mov gs,ax					; GS = B800h
ini:
    ;call clear 
    
    mov cx,len1
    mov si,0
    mov di,960

    print1:
    mov al,byte[msg1+si]
    inc si
    mov [gs:di],ax
    add di,2
    loop print1

    mov cx,len2
    mov si,0
    mov di,1120

    print2:
    mov al,byte[msg2+si]
    inc si
    mov [gs:di],ax
    add di,2
    loop print2

switch:
    mov ah, 0x01
	int 16h
	jz ini
    mov ah, 0
    int 16h
    cmp al, '1'
    je _prog1   
    cmp al, '2'
    je _prog2   
    cmp al, '3'
    je _prog3   
    cmp al, '4'
    je _prog4  
    cmp al, '5'
    je _prog5
    jmp ini
_prog1:
    call prog1
    jmp ini
_prog2:
    call prog2
    jmp ini
_prog3:
    call prog3
    jmp ini
_prog4:
    call prog4
    jmp ini
_prog5:
    call prog1
    call prog2
    call prog3
    call prog4
    jmp ini
clear:       

    mov ax, 0003h
    int 10h          

    ret

Datedef:
    msg1 db '18340178 XYP'
    len1 equ ($-msg1)

    msg2 db 'Press 1~5 to select program:'
    len2 equ ($-msg2)

    cmd db '1234' ;ִ��˳��
times 510-($-$$) db 0
    db 0x55,0xaa
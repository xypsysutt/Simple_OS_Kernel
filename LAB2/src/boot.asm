org  7C00h    

prog1 equ 08300h    ;tl
prog2 equ 08700h    ;tr
prog3 equ 08B00h    ;bl
prog4 equ 08F00h    ;br
mon equ 08100h

%macro read_secotr 6  ;(offset,??????????????????????????????????????)
	;????�???
	mov ax,cs
	mov es,ax
	;????????????
	mov bx,%1
	mov ah,2				  ;?????
	mov al,%2				  ;??????
    mov dl,%3                 ;???????? ; ?????0??????U???80H
    mov dh,%4                 ;????? ; ???????0
    mov ch,%5                 ;????? ; ???????0
    mov cl,%6                 ;????????? ; ???????1
    int 13H ;                ?????????BIOS??13h????
%endmacro
    mov ax,cs
	mov es,ax					; ES = 0
	mov ds,ax					; DS = CS
	mov es,ax					; ES = CS
	mov ax,0B800h				; 閿熶茎鎲�?嫹閿熸枻鎷烽敓鏂ゆ�?�閿熺殕杈炬嫹閿熸枻鎷峰��??敓鏂ゆ�?�鍧�?
	mov gs,ax					; GS = B800h
start:
    mov cx,msglen
    mov si,0
    mov di,960

    print1:
    mov al,byte[message+si]
    inc si
    mov [gs:di],ax
    add di,2
    loop print1

    mov word[i],0
    mov si,0
    read_secotr mon,1,0,0,0,2
    mov si,word[num_prog]
    mov word[prog_pos+si],mon
    inc word[num_prog]

    read_secotr prog1,2,0,0,0,3
    mov si,word[num_prog]
    mov word[prog_pos+si],prog1
    inc word[num_prog]

    read_secotr prog2,2,0,0,0,5
    mov si,word[num_prog]
    mov word[prog_pos+si],prog2
    inc word[num_prog]

    read_secotr prog3,2,0,0,0,7
    mov si,word[num_prog]
    mov word[prog_pos+si],prog3
    inc word[num_prog]

    read_secotr prog4,2,0,0,0,9
    mov si,word[num_prog]
    mov word[prog_pos+si],prog4
    inc word[num_prog]

    mov cx,len1
    mov si,0
    mov di,0
    mov ah,17h
    print2:
    mov al,byte[msg1+si]
    inc si
    mov [gs:di],ax
    add di,2
    loop print2

    mov al,byte[num_prog]
    add ax,'0'
    mov [gs:di],ax
    
    mov ah, 0x00
	int 16h
    


    ;read_secotr prog5,1,0,0,0,7


    jmp mon
    ;jmp 0x0000:0xA300
    jmp $ ;?????�??????????
Datedef:
    message db 'Booting ...'
    msglen equ ($-message)
    msg1 db 'num of prog :'
    len1 equ ($-msg1)

    i db 0
    num_prog dw 0
    prog_pos dw 0
times 510-($-$$) db 0  
dw  0xaa55
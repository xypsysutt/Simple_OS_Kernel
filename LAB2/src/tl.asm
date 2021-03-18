;   NASM汇编格式
;   实验一
     Dn_Rt equ 1                  ;D-Down,U-Up,R-right,L-Left
     Up_Rt equ 2                  ;
     Up_Lt equ 3                  ;
     Dn_Lt equ 4                  ;
     delay equ 50000					; 计时器延迟�?�数,用于控制画�?�的速度
     ddelay equ 580					; 计时器延迟�?�数,用于控制画�?�的速度

     ;org 7c00h					; 程序加载�?100h，可用于生成COM/7c00H引�?�扇区程�?
     org 08300h 
   screen_left equ -1     
    screen_top equ -1    
    screen_right equ 40   
    screen_bottom equ 13  

start:

	;xor ax,ax					; AX = 0   程序加载�?0000�?100h才能正确执�??
    call clear
    mov ax,cs
	mov es,ax					; ES = 0
	mov ds,ax					; DS = CS
	mov es,ax					; ES = CS
	mov ax,0B800h				; 文本窗口显存起�?�地址
	mov gs,ax					; GS = B800h

loop1:
	dec word[count]				; 递减计数变量
	jnz loop1					; >0：跳�?;
	mov word[count],delay
	dec word[dcount]				; 递减计数变量
      jnz loop1
	mov word[count],delay
	mov word[dcount],ddelay
      


	mov ah, 0x01
	int 16h
	jz ini
	mov ah, 0x00
	int 16h
	cmp ax,2c1ah
    je Quit

ini:  
	mov al,1
      cmp al,byte[rdul]
	jz  DnRt
      mov al,2
      cmp al,byte[rdul]
	jz  UpRt
      mov al,3
      cmp al,byte[rdul]
	jz  UpLt
      mov al,4
      cmp al,byte[rdul]
	jz  DnLt
      jmp $	

DnRt:
    inc word[x]
    inc word[y]
    mov bx,word[x]
    mov ax,screen_bottom
    sub ax,bx
    jz  dr2ur
    mov bx,word[y]
    mov ax,screen_right
    sub ax,bx
    jz  dr2dl
    jmp show

dr2ur:
    mov word[x],screen_bottom-2
    mov byte[rdul],Up_Rt
    jmp show

dr2dl:
    mov word[y],screen_right-2
    mov byte[rdul],Dn_Lt
    jmp show


UpRt:
    dec word[x]
    inc word[y]
    mov bx,word[y]
    mov ax,screen_right
    sub ax,bx
    jz  ur2ul
    mov bx,word[x]
    mov ax,screen_top
    sub ax,bx
    jz  ur2dr
    jmp show

ur2ul:
    mov word[y],screen_right-2
    mov byte[rdul],Up_Lt
    jmp show

ur2dr:
    mov word[x],screen_top+2
    mov byte[rdul],Dn_Rt
    jmp show


UpLt:
    dec word[x]
    dec word[y]
    mov bx,word[x]
    mov ax,screen_top
    sub ax,bx
    jz  ul2dl
    mov bx,word[y]
    mov ax,screen_left
    sub ax,bx
    jz  ul2ur
    jmp show

ul2dl:
    mov word[x],screen_top+2
    mov byte[rdul],Dn_Lt
    jmp show
ul2ur:
    mov word[y],screen_left+2
    mov byte[rdul],Up_Rt
    jmp show

DnLt:
    inc word[x]
    dec word[y]
    mov bx,word[y]
    mov ax,screen_left
    sub ax,bx
    jz  dl2dr
    mov bx,word[x]
    mov ax,screen_bottom
    sub ax,bx
    jz  dl2ul
    jmp show

dl2dr:
    mov word[y],screen_left+2
    mov byte[rdul],Dn_Rt
    jmp show

dl2ul:
    mov word[x],screen_bottom-2
    mov byte[rdul],Up_Lt
    jmp show


show:
	
    xor ax,ax
    mov ax,word[x]
    mov bx,80
    mul bx
    add ax,word[y]
    mov bx,2
    mul bx
    mov bx,ax
    
    mov cx,5
    mov si,0
    loop11:
        add ah,1
        cmp ah,0fh
        jnz pp2
        mov ah,1
        pp2:
        mov al,byte[stuNumber+si]
        mov [gs:bx],ax
        sub bx,162
        inc si
        loop loop11
    jmp loop1
	
end:
    jmp $                   ; 停�?�画框，无限�?�? 
Quit:
    ;jmp 0a100h
    call clear
    ret
clear:
	mov ax,0B800h
	mov es,ax
	mov si,0
	mov cx,80*25
	mov dx,0
	_clear:
		mov [es:si],dx
		add si,2
	loop _clear
	ret
datadef:	
    count dw delay
    dcount dw ddelay
    rdul db 2         ; 向右下运�?
    x    dw 1
    y    dw 1
    stuName db 'ctrl + z to quit '
    msglen dw ($-stuName)
    stuNumber  db 'Prog1'

  times 1022-($-$$) db 0
                   db 0x55,0xaa

BITS 16
org  7C00h    
PTB equ 07E00h       ;PTB
mon equ 08000h      ;mon
%macro read_secotr 6  ;(offset,扇区号,驱动器号，磁头号，柱面号，起始扇区号)
	mov ax,cs
	mov es,ax
	mov bx,%1
	mov ah,2				 
	mov al,%2			;扇区数	  
    mov dl,%3           ;驱动器号      
    mov dh,%4           ;磁头号 
    mov ch,%5           ;柱面号    
    mov cl,%6           ;起始扇区号      
    int 13H            
%endmacro

global _start
_start:
    mov ax,cs
	mov es,ax					; ES = 0
	mov ds,ax					; DS = CS
	mov es,ax					; ES = CS
	mov ax,0B800h				; 显存
	mov gs,ax					; GS = B800h
    
    mov cx,msglen
    mov si,0
    mov di,960
    print1:
    mov al,byte[message+si]
    inc si
    mov [gs:di],ax
    add di,2
    loop print1

    read_secotr PTB,1,0,0,0,2       ;占1个扇区 
    read_secotr mon,16,0,0,0,3      ;占16个扇区

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
    mov ax,cs
	mov ds,ax
	;SetTimer
	mov al,34h
	out 43h,al ; write control word
	mov ax,1193182/20	;X times / seconds
	out 40h,al
	mov al,ah
	out 40h,al

    mov ah, 0x00        ;任意键继续
	int 16h

    jmp mon
Datedef:
    message db 'X-OS is Booting ...\n'
    msglen equ ($-message)
    msg1 db 'Done.Press any key to continue'
    len1 equ ($-msg1)

    i db 0
    num_prog dw 0
    prog_pos dw 0
times 510-($-$$) db 0  
dw  0xaa55
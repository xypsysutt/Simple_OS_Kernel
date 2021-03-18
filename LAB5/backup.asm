BITS 16
global SAVE
global RESTART

SAVE:
    ;cli
    ;Stack: */fg/cs/ip
    
    push ds
    ;Stack: */fg/cs/ip/ds
    push cs
    ;Stack: */fg/cs/ip/ds/cs

    ; to get cs
    pop ds
    ;Stack: */fg/cs/ip/ds
    pop word[ds:_DS]
    ;Stack: */fg/cs/ip/
    mov [ds:_AX],ax 
    mov [ds:_BX],bx 
    mov [ds:_CX],cx 
    mov [ds:_DX],dx 

    mov ax,es
    mov [ds:_ES],ax 
     
    mov ax,di
    mov [ds:_DI],ax 
    mov ax,si 
    mov [ds:_SI],ax 
    mov ax,bp 
    mov [ds:_BP],ax
    mov ax,sp
    mov [ds:_SP],ax
    mov ax,ss
    mov [ds:_SS],ax
    ;Stack: */fg/cs/ip
    pop word[ds:_IP]
    pop word[ds:_CS]
    pop word[ds:_FG]
    ;Stack: */
    ;go back
    Timer:
    
    mov ax,cs
    mov ds,ax                   ; DS = CS
    mov	ax,0B800h               ; 文本窗口显存起始地址
    mov	gs,ax                   ; GS = B800h
    mov ah, 0Fh 
    dec byte [count]          
    jnz end            

    mov ah,08h

    mov byte[count],delay      
    mov si, mark            
    add si, [offset]     
    mov al, [si]                             
    mov [gs:((80*24+79)*2)], ax ; 更新显存
    inc byte[offset]      
    cmp byte[offset], 3   
    jne end                  
    mov byte[offset], 0   
end:

RESTART:

    mov sp,[ds:_SP]
    pop ax
    pop ax
    pop ax
    ;stand pos /*/*/*/
    
    mov ss,[ds:_SS]
    mov ax,[ds:_FG]
    push ax
    ;Stack: */fg/
    mov ax,[ds:_CS]
    push ax
    ;Stack: */fg/cs/
    mov ax,[ds:_IP]
    push ax
    ;Stack: */fg/cs/ip/
    mov es,[ds:_ES]
    mov di,[ds:_DI]
    mov si,[ds:_SI]
    mov bp,[ds:_BP]
    mov dx,[ds:_DX]
    mov cx,[ds:_CX]
    mov bx,[ds:_BX]
    mov ax,[ds:_DS]
    push ax
    ;Stack: */fg/cs/ip/ds/
    mov ax,[ds:_AX]
    pop ds
    ;Stack: */fg/cs/ip/
    push ax
    mov al,20h                  ; AL = EOI
    out 20h,al                  ; 发送EOI到主8529A
    out 0A0h,al                 ; 发送EOI到从8529A
    pop ax
    
    ;sti
    iret



DataArea:
    delay equ 3                 
    count db delay              
    mark db '\|/'         
    offset dw 0 
REGDATA:
    _AX dw 0
    _BX dw 0
    _CX dw 0
    _DX dw 0

    _ES dw 0
    _DS dw 0
    _DI dw 0
    _SI dw 0
    _BP dw 0
    _SP dw 0
    _SS dw 0
    _IP dw 0
    _CS dw 0
    _FG dw 0
    



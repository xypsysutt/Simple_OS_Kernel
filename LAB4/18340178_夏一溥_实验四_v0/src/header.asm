; 写中断向量表 (中断号，中断处理程序地址)
%macro WriteIVT 2
    pusha
	mov ax,0
	mov es,ax
	mov ax,%1
	mov bx,4
	mul bx
	mov si,ax
	mov ax,%2
	mov [es:si],ax ; offset
	add si,2
	mov ax,cs
	mov [es:si],ax
    popa
%endmacro
; 保护原始中断，中断写中断 
; (dest，src)
%macro SwitIVT 2        
    pusha
    mov ax, 0
    mov es, ax
    mov si, [es:%2*4]
    mov [es:%1*4], si
    mov si, [es:%2*4+2]
    mov [es:%1*4+2], si
    popa
%endmacro
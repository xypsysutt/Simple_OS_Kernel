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

%macro PRINTLINE 4       ; 在指定位置打印字符串；参数：（串地址，串长，行号，列号）
    pusha                   
    push ds
    push es
    mov	ax, cs              ; 置其他段寄存器值与CS相同
    mov	ds, ax              ; 数据段
    mov	bp, %1              ; BP=当前串的偏移地址
    mov	ax, ds              ; ES:BP = 串地址
    mov	es, ax              ; 置ES=DS
    mov	cx, %2              ; CX = 串长（=9）
    mov	ax, 1301h           ; AH = 13h（功能号）、AL = 01h（光标置于串尾）
    mov	bx, 0007h           ; 页号为0(BH = 0) 黑底白字(BL = 07h)
    mov dh, %3              ; 行号=0
    mov	dl, %4              ; 列号=0
    int	10h                 ; BIOS的10h功能：显示一行字符
    pop es
    pop ds
    popa                    
%endmacro
BITS 16

global _CLS
global _putchar
global _printLine
global _getchar
global _execute

; parament ()
_CLS:               ; clear the screen
    push ax
    mov ax, 0003h
    int 10h        
    pop ax
    retf
; parament (char)
_putchar:                   
    pusha
    mov bp, sp
    add bp, 20           
    mov al, [bp]           ; al=要打印的字符
    mov bh, 0              ; bh=页码
    mov ah, 0Eh            ; 功能号：打印一个字符
    int 10h                ; 打印字符
    popa
    retf
; parament ()
_getchar:
    mov ah, 0              ; 功能号
    int 16h                ; 读取字符，al=读到的字符
    mov ah, 0              
    retf
; parament (char *msg,short len,short X,int Y)
_printLine:                ; 指定位置打印字符串
    pusha                  
    mov si, sp             
    add si, 20           
    mov	ax, cs             
    mov	ds, ax   
    mov	bp, [si]           
    mov	ax, ds             
    mov	es, ax             
    mov	cx, [si+4]         
    mov	ax, 1301h          ; AH = 13h（功能号）、AL = 01h（光标置于串尾）
    mov	bx, 0007h          ; 页号为0(BH = 0) 黑底白字(BL = 07h)
    mov dh, [si+8]         ; 行号=0
    mov	dl, [si+12]        ; 列号=0
    int	10h                ; BIOS的10h功能：显示一行字符
    popa                    
    retf
; parament (short sec,short addr)
_execute:
    pusha
    mov bp, sp
    add bp, 20          
    mov ax,cs              
    mov es,ax               
    mov bx, [bp+4]       
    mov ah,2               
    mov al,2         
    mov dl,0          
    mov dh,1            ;head          
    mov ch,0           
    mov cl,[bp]         
    int 13H                
    call dword cs_ip    ;必须手动入栈操作
    cs_ip:              ;否则如使用call会因为c语言callret为32位导致不能返回 
    mov si, sp             
    mov word[si], ip_new 
    jmp [bp+4]
    ip_new:
    popa
    retf

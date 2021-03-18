BITS 16
[extern OS]
%include "header.asm"
global _start
_start:  
    WriteIVT 08h, Timer
    call dword OS 
    jmp _start    

Timer:
    push ax
    push ds
    push gs
    push si
    mov ax,cs
    mov ds,ax                   ; DS = CS
    mov	ax,0B800h               ; 文本窗口显存起始地址
    mov	gs,ax                   ; GS = B800h
    mov ah, 0Fh 
    dec byte [count]          
    jnz end            

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
    mov al,20h                  ; AL = EOI
    out 20h,al                  ; 发送EOI到主8529A
    out 0A0h,al                 ; 发送EOI到从8529A

    pop si
    pop gs
    pop ds
    pop ax
    iret                        ; 从中断返回


DataArea:
    delay equ 3                 
    count db delay              
    mark db '\|/'         
    offset dw 0           
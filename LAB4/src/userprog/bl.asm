org 01000h

    Dn_Rt equ 1            ; D-Down,U-Up,R-right,L-Left
    Up_Rt equ 2
    Up_Lt equ 3
    Dn_Lt equ 4
    delay equ 50000        ; 计时器延迟计数,用于控制画框的速度
    ddelay equ 580         ; 计时器延迟计数,用于控制画框的速度

    screen_left equ -1     ; 字符运动左边界
    screen_top equ 11      ; 字符运动上边界
    screen_right equ 40    ; 字符运动右边界
    screen_bottom equ 25   ; 字符运动下边界
    originpos_y equ 0      ; 起点列数
    originpos_x equ 19     ; 起点行数

start:
    pusha
    call ClearScreen       ; 清屏
    mov ax,cs
    mov es,ax              ; ES = CS
    mov ds,ax              ; DS = CS
    mov es,ax              ; ES = CS
    mov ax,0B800h
    mov gs,ax              ; GS = B800h，指向文本模式的显示缓冲区
    mov byte[char],'3'


initialize:                ; 多次调用用户程序时，可保证初始值是相同的
    mov word[x], originpos_x
    mov word[y], originpos_y
    mov word[count], delay
    mov word[dcount], ddelay
    mov byte[rdul], Dn_Rt  ; 向右下运动

loop1:
    dec word[count]        ; 递减计数变量
    jnz loop1              ; >0：跳转;
    mov word[count],delay
    dec word[dcount]       ; 递减计数变量
    jnz loop1
    mov word[count],delay
    mov word[dcount],ddelay

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
    ; jmp $

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
    xor ax,ax              ; 计算显存地址
    mov ax,word[x]
    mov bx,80
    mul bx
    add ax,word[y]
    mov bx,2
    mul bx
    mov bp,ax
    mov ah,3     
skip:
    mov al,byte[char]      
    mov word[gs:bp],ax     

    mov ah, 01h           
    int 16h
    jz continue            
    mov ah, 0             
    int 16h
    cmp ax,2c1ah           
    je quit         

continue:
    jmp loop1

end:
    jmp $                  ; 停止画框，无限循环

quit:
    popa
    retf

ClearScreen:               ; 函数：清屏
    pusha
    mov ax, 0003h
    int 10h                ; 中断调用，清屏
    popa
    ret

DataArea:
    count dw delay
    dcount dw ddelay
    rdul db Dn_Rt          ; 向右下运动
    char db 0

    x dw originpos_x
    y dw originpos_y   

    times 1024-($-$$) db 0

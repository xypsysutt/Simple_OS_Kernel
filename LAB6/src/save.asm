BITS 16

global INT08H_START
global INT20H_START
global INT21H_START
global INT22H_START
global shell_mode
global _preload
;*********宏汇编过程SAVE****************
%macro SAVE 1
    cli
    ;Stack: */fg/cs/ip
    
    push ds
    ;Stack: */fg/cs/ip/ds
    push cs
    ;Stack: */fg/cs/ip/ds/cs

    ; to get cs
    pop ds
    ;Stack: */fg/cs/ip/ds
    pop word[ds:_DS_]
    ;Stack: */fg/cs/ip/
    mov [ds:_AX_],ax 
    mov [ds:_BX_],bx 
    mov [ds:_CX_],cx 
    mov [ds:_DX_],dx 

    mov ax,es
    mov [ds:_ES_],ax 
     
    mov ax,di
    mov [ds:_DI_],ax 
    mov ax,si 
    mov [ds:_SI_],ax 
    mov ax,bp 
    mov [ds:_BP_],ax
    mov ax,sp
    mov [ds:_SP_],ax
    mov ax,ss
    mov [ds:_SS_],ax
    ;Stack: */fg/cs/ip
    pop word[ds:_IP_]
    pop word[ds:_CS_]
    pop word[ds:_FG_]
    ;Stack: */
    ;go call int

    mov ax,[ds:_AX_]
    call %1

    RESTART%1:

    mov sp,[ds:_SP_]
    pop ax
    pop ax
    pop ax
    ;stand pos /*/*/*/
    
    mov ss,[ds:_SS_]
    mov ax,[ds:_FG_]
    push ax
    ;Stack: */fg/
    mov ax,[ds:_CS_]
    push ax
    ;Stack: */fg/cs/
    mov ax,[ds:_IP_]
    push ax
    ;Stack: */fg/cs/ip/
    mov es,[ds:_ES_]
    mov di,[ds:_DI_]
    mov si,[ds:_SI_]
    mov bp,[ds:_BP_]
    mov dx,[ds:_DX_]
    mov cx,[ds:_CX_]
    mov bx,[ds:_BX_]
    mov ax,[ds:_DS_]
    push ax
    ;Stack: */fg/cs/ip/ds/
    mov ax,[ds:_AX_]
    pop ds
    ;Stack: */fg/cs/ip/
    push ax
    mov al,20h                  ; AL = EOI
    out 20h,al                  ; 发送EOI到主8529A
    out 0A0h,al                 ; 发送EOI到从8529A
    pop ax
    
    sti
    iret
%endmacro
;*******************************************

;*********时钟中断***************************
INT08H_START:                             ; 08h号时钟中断处理程序
    call show_hot_whell
    cmp word[cs:shell_mode], 0
    je quit_timer
    ;Stack: */fg/cs/ip
    push sp
    ;Stack: */fg/cs/ip/sp/
    push di
    push ax
    ;Stack: */fg/cs/ip/sp/di/ax
    push ds
    push cs
    pop ds
    ;Stack: */fg/cs/ip/sp/di/ax/ds
    mov ax,pcbSize
    mul word[cs:run_now]
    mov di,ax
    mov ax,ss
    mov word[ds:_SS+di],ax
    mov ax,gs
    mov word[ds:_GS+di],ax
    mov ax,fs
    mov word[ds:_FS+di],ax
    mov ax,es
    mov word[ds:_ES+di],ax
    mov ax,si
    mov word[ds:_SI+di],ax
    mov ax,bp
    mov word[ds:_BP+di],ax  
    mov ax,bx
    mov word[ds:_BX+di],ax
    mov ax,cx
    mov word[ds:_CX+di],ax
    mov ax,dx
    mov word[ds:_DX+di],ax
    ;Stack: */fg/cs/ip/sp/di/ax/ds
    pop ax
    ;Stack: */fg/cs/ip/sp/di/ax/
    mov word[ds:_DS+di],ax
    pop ax
    mov word[ds:_AX+di],ax
    pop ax
    mov word[ds:_DI+di],ax
    pop ax
    mov word[ds:_SP+di],ax
    ;Stack: */fg/cs/ip/
    pop ax
    mov word[ds:_IP+di],ax
    pop ax
    mov word[ds:_CS+di],ax
    pop ax
    mov word[ds:_FG+di],ax
    ;Stack: */

    call schedule               ; 进程调度

reg_load:                       
    mov ax,pcbSize
    mul word[cs:run_now]
    mov di,ax
    ;Stack: */
    mov ax, [cs:_AX+di]
    mov bx, [cs:_BX+di]
    mov cx, [cs:_CS+di]
    mov dx, [cs:_DX+di]

    mov sp, [cs:_SP+di]
    mov bp, [cs:_BP+di]
    mov ds, [cs:_DS+di]
    mov es, [cs:_ES+di]
    mov fs, [cs:_FS+di]
    mov gs, [cs:_GS+di]
    mov ss, [cs:_SS+di]
    add sp, 6                   

    push word[cs:_FG+di]            ; 新进程flags
    push word[cs:_CS+di]            ; 新进程cs
    push word[cs:_IP+di]            ; 新进程ip

    ;Stack: */fg/cs/ip/
    push word[cs:_DI+di]
    pop di                         

quit_timer:
    push ax
    mov al, 20h
    out 20h, al
    out 0A0h, al
    pop ax
    iret

schedule:                       
    pusha
    mov ax,pcbSize
    mul word[cs:run_now]
    mov di,ax

    mov byte[cs:_STATE+di], 1         

    mov ah, 01h                    
    int 16h
    jz judge_num               
    mov ah, 0                    
    int 16h
    cmp al, 27                    
    jne judge_num               

    mov word[cs:run_now], 0
    mov word[cs:shell_mode], 0     
    call shutdown
    jmp quit_schedule

    judge_num:                  
        inc word[cs:run_now]
        add di, pcbSize                
        cmp word[cs:run_now], 10            ;总共最多10个进程
        jna change_state        
        mov word[cs:run_now], 1
        mov di, pcbSize       
    change_state:
        cmp byte[cs:_STATE+di], 1      
        jne judge_num           
        mov byte[cs:_STATE+di], 2      
    quit_schedule:
    popa
    ret

;*****************预加载到分时系统内存*********
_preload:                  
    pusha
    mov bp,sp
    add bp,20
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

    mov ax,pcbSize
    mul word[bp+8]
    mov di,ax

    mov ax,[bp+8]
    mov byte[cs:_ID+di],al
    mov ax,0
    mov word[cs:_DS+di],ax
    mov word[cs:_ES+di],ax
    mov word[cs:_FS+di],ax
    mov word[cs:_SS+di],ax
    mov word[cs:_CS+di],ax
    mov byte[cs:_STATE+di],1
    mov ax,[bp+4]
    mov word[cs:_IP+di],ax

    popa
    retf
;*************关闭程序就绪态，在一次调度后退回内核***************
shutdown:
    pusha
    mov cx, 10                      
    mov si, pcb0+pcbSize
    loop1:
        mov byte[cs:si+_STATE], 0      
        add si, 34                
        loop loop1
    popa
    ret
;********************显式风火轮过程独立*********************
show_hot_whell:
    pusha
    push ds
    push gs
    mov ax,cs
    mov ds,ax                   ; DS = CS
    mov	ax,0B800h               ; 文本窗口显存起始地址
    mov	gs,ax                   ; GS = B800h
    mov ah, 0Fh 
    dec byte [count]          
    jnz end_whell            

    mov ah,03h

    mov byte[count],delay      
    mov si, mark            
    add si, [offset]     
    mov al, [si]                             
    mov [gs:((80*24+79)*2)], ax ; 更新显存
    inc byte[offset]      
    cmp byte[offset], 3   
    jne end_whell                 
    mov byte[offset], 0
    
end_whell:
    pop gs
    pop ds
    popa
    ret

;*********************以下与实验5无区别*********************
INT20H_START:
    pop word[_tmp]
    pop word[_tmp]
    pop word[_tmp]
    retf

[extern SYS_ShowTime]
[extern SYS_reBoot]
[extern SYS_PowerOff]

INT21H: ;8838
    mov si, cs
    mov ds, si                ; ds = cs
    mov si, ax
    shr si, 8                 ; si = 功能号
    add si, si                ; si = 2 * 功能号
    call [SYSINT+si]       ; 系统调用函数
    ret
    SYSINT:
        dw SYS_ShowTime
        dw SYS_reBoot
        dw SYS_PowerOff
INT21H_START:
    SAVE INT21H

INT22H:
    mov ax,cs
	mov es,ax					; ES = 0
	mov ds,ax					; DS = CS
	mov es,ax					; ES = CS
	mov ax,0B800h				; 显存
	mov gs,ax					; GS = B800h
    
    mov cx,msglen_INT22H
    mov si,0
    mov di,960
    printINT22H:
    mov al,byte[message_INT22H+si]
    inc si
    mov ah,06h
    mov [gs:di],ax
    add di,2
    loop printINT22H
    ret
INT22H_START:
    SAVE INT22H
REGDATA:
    _AX_ dw 0
    _BX_ dw 0
    _CX_ dw 0
    _DX_ dw 0

    _ES_ dw 0
    _DS_ dw 0
    _DI_ dw 0
    _SI_ dw 0
    _BP_ dw 0
    _SP_ dw 0
    _SS_ dw 0
    _IP_ dw 0
    _CS_ dw 0
    _FG_ dw 0

    _tmp dw 0
shell_mode dw 0
run_now dw 0
;********************内存申请宏*************************
%macro prog_contr_block 1       ; 参数：段值
    %1_AX dw 0                           
    %1_CX dw 0                           
    %1_DX dw 0                           
    %1_BX dw 0                           
    %1_SP dw 0                      
    %1_BP dw 0                           
    %1_SI dw 0                           
    %1_DI dw 0                           
    %1_DS dw 0                           
    %1_ES dw 0                           
    %1_FS dw 0                           
    %1_GS dw 0                      
    %1_SS dw 0                           
    %1_IP dw 0                           
    %1_CS dw 0                           
    %1_FG dw 0                         
    %1_ID db 0                           
    %1_STATE db 0                        ;0-初始 1-就绪 2-运行
%endmacro

DataArea:
    delay equ 3                 
    count db delay              
    mark db '\|/'         
    offset dw 0 

    message_INT22H db 'INT22H'
    msglen_INT22H equ ($ - message_INT22H)

pcbSize equ pcb1 - pcb0
pcb0: 
    _AX dw 0                           
    _CX dw 0                           
    _DX dw 0                           
    _BX dw 0                           
    _SP dw 0                      
    _BP dw 0                           
    _SI dw 0                           
    _DI dw 0                           
    _DS dw 0                           
    _ES dw 0                           
    _FS dw 0                           
    _GS dw 0                      
    _SS dw 0                           
    _IP dw 0                           
    _CS dw 0                           
    _FG dw 0                         
    _ID db 0                           
    _STATE db 0  

;****************PCB表*************************
pcb1:
prog_contr_block _1
prog_contr_block _2
prog_contr_block _3
prog_contr_block _4
prog_contr_block _5
prog_contr_block _6
prog_contr_block _7
prog_contr_block _8
prog_contr_block _9
prog_contr_block _10
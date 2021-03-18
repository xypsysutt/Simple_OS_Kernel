PCB:
    db 'Name   |Sector  |Size   |Addr   ',0xd,0xa,0x0
    db 'Prog1  |1       |1024   |0A300h ',0xd,0xa,0x0
    db 'Prog2  |3       |1024   |0A700h ',0xd,0xa,0x0
    db 'Prog3  |5       |1024   |0AB00h ',0xd,0xa,0x0
    db 'Prog4  |7       |1024   |0AF00h ',0xd,0xa,0x0
Num:
    db 4

times 512-($-$$) db 0    

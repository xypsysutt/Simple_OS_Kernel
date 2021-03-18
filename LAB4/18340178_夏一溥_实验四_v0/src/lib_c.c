/*
prog1 equ 08300h    ;tl
prog2 equ 08700h    ;tr
prog3 equ 08B00h    ;bl
prog4 equ 08F00h    ;br
*/
typedef unsigned char u8;   //1字节
typedef unsigned short u16; //2字节
extern void _printLine(char *msg, u16 len, u16 row, u16 col);
extern void _putchar(char c);
extern char _getchar();
extern void _CLS();
extern void _execute(short sec,short addr);
void print(char* str) {
    int cnt=0;
    while(str[cnt++]!='\0');
    cnt--;
    for(int i = 0, len = cnt; i < len; i++) {
        _putchar(str[i]);
    }
}
void help(){
    char* cue = "X-OS with NASM&&C\r\n";
    char* author = "XYP 18340178\r\n";
    char* help = 
    "print number to select function\r\n"
    "1 - prog1\r\n"
    "2 - prog2\r\n"
    "3 - prog3\r\n"
    "4 - prog4\r\n"
    "5 - Sequential execution\r\n"
    "6 - list the user program\r\n"
    ">>> ";
    _CLS();
    print(cue);
    print(author);
    print(help);
    return;
}


void OS(){
    char cmd='\0';
    u16 addr[5]={0,0x1000,0x1000,0x1000,0x1000};
    while(1){
        _CLS();
        help();
        cmd=_getchar();
        _putchar(cmd);
        cmd-='0';
        if(cmd<=4){
            u16 sec = 1 + (cmd-1)*2; 
            _execute(sec,addr[cmd]);
        }
        else if(cmd == 5){
            for(int i=1;i<5;i++){
                u16 sec = 1 + (i - 1)*2;
                _execute(sec,addr[i]);
            }
        }
        else if(cmd == 6){
            _CLS();
            char* PCB=
            "Name   |Sector  |Size   |Addr    \r\n"
            "Prog1  |1       |1024   |0xA300h \r\n"
            "Prog2  |3       |1024   |0xA700h \r\n"
            "Prog3  |5       |1024   |0xAB00h \r\n"
            "Prog4  |7       |1024   |0xAF00h \r\n";
            print(PCB);
            _getchar();
        }
        else{
            _CLS();
            _getchar();
            char *warn ="Press any key to quit\r\n";
            _printLine(warn,24,0,0);
            return;
        }
    }   
}

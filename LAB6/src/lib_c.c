/*
prog1 equ 08300h    ;tl
prog2 equ 08700h    ;tr
prog3 equ 08B00h    ;bl
prog4 equ 08F00h    ;br
*/
#include <stdarg.h>
typedef unsigned char u8;   //1字节
typedef unsigned short u16; //2字节
extern void _printLine(char *msg, u16 len, u16 row, u16 col);
extern void _putchar(char c);
extern char _getchar();
extern void _CLS();
extern void _execute(short sec,short addr);
extern void _preload(short sec,short addr,short PID);
extern u8 _getYear();
extern u8 _getMonth();
extern u8 _getDay();
extern u8 _getHour();
extern u8 _getMinute();
extern u8 _getSecond();
extern void SYS_PowerOff();
void pause();
void myPrintf(char *s, ...);				// 需要实现的目标函数
void printNum(unsigned long num, int base); // 通用数字打印函数 
void printDeci(int dec);					// 打印十进制数
extern u8 run_now;
//extern u8 run_num;
extern u8 shell_mode;
char getchar(){
	// 获得一个按键（需等待）
	char ch;
	asm volatile("int 0x16;"
			:"=a"(ch)
			:"a"(0x1000)
			);
	return ch;
}
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
    "7 - syscall\r\n"
    "8 - PowerOff\r\n"
    "9 - RR-Time sharing system\r\n"
    "X-OS >> ";
    _CLS();
    print(cue);
    print(author);
    print(help);
    return;
}


void OS(){
    char cmd='\0';
    u16 addr[6]={0,0x1000,0x2000,0x3000,0x4000,0x5000};
    while(1){
        _CLS();
        help();
        cmd=_getchar();
        //cmd = '1';
        _putchar(cmd);
        _getchar();
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
            "Prog1  |1       |1024   |0x01000h \r\n"
            "Prog2  |3       |1024   |0x02000h \r\n"
            "Prog3  |5       |1024   |0x03000h \r\n"
            "Prog4  |7       |1024   |0x04000h \r\n"
            "syscall|9       |1024   |0x05000h \r\n";
            print(PCB);
            _getchar();
        }
        else if(cmd == 7){
            _execute(9,addr[5]);
        }
        else if(cmd == 8){
            SYS_PowerOff();
        }
        else if(cmd == 9){
            /*_CLS();
            char str[30]="This is my printf\r\n";
            int a = 178;
            char ch = 'A';
            printf("ch=%c, a=%d, str=%s", ch, a, str);
            _getchar();*/
            for(int i=1;i<5;i++){
                u16 sec = 1 + (i - 1)*2;
                _preload(sec,addr[i],i);
            }
            //run_num=4;
            shell_mode=1;
            //run_now=1;
            pause();
            shell_mode=0;
            _CLS();
        }
        else{
            _CLS();
            _getchar();
            char *warn ="Press any key to quit\r\n";
            _printLine(warn,24,0,0);
        }
    }   
}

/*
 *更新于实验5，实现时间打印
 */
u8 bcd2decimal(u8 bcd)
{
    return ((bcd & 0xF0) >> 4) * 10 + (bcd & 0x0F);
}
char* itoa(int val) {
	if(val==0) return "0";
	static char buf[32] = {0};
	int i = 30;
	for(; val && i ; --i, val /= 10) {
		buf[i] = "0123456789"[val % 10];
    }
	return &buf[i+1];
}
/* 显示日期时间 */
void _ShowTime() {
    _putchar('\r');
    _putchar('\n');
    _putchar('2'); 
    _putchar('0');
    print(itoa(bcd2decimal(_getYear()))); 
    _putchar('-');
    print(itoa(bcd2decimal(_getMonth()))); 
    _putchar('-');
    print(itoa(bcd2decimal(_getDay()))); 
    _putchar(' ');
    print(itoa(bcd2decimal(_getHour())));
    _putchar(':');
    print(itoa(bcd2decimal(_getMinute()))); 
    _putchar(':');
    print(itoa(bcd2decimal(_getSecond())));
    _putchar('\r');
    _putchar('\n');
    return;
}

void myPrintf(char *s, ...)
{
    int i = 0;

	/* 可变参第一步 */
    va_list va_ptr;

	/* 可变参第二部 */
    va_start(va_ptr, s);

	/* 循环打印所有格式字符串 */
    while (s[i] != '\0')
    {
		/* 普通字符正常打印 */
		if (s[i] != '%')
		{
    	    _putchar(s[i++]);
			continue;
		}
		
		/* 格式字符特殊处理 */
		switch (s[++i])   // i先++是为了取'%'后面的格式字符
		{
			case 'd': printDeci(va_arg(va_ptr,int));           
			  		  break; 
		    case 'c': _putchar(va_arg(va_ptr,int));          
			  		  break;
		    case 's': print(va_arg(va_ptr,char *));
					  break;
			default : break;
		}

		i++; // 下一个字符
    }

	/* 可变参最后一步 */
    va_end(va_ptr);
}

void printDeci(int dec)
{
    int num;

    /* 处理有符号整数为负数时的情况 */
	if (dec < 0)
    {
        _putchar('-');
		dec = -dec;  	   
    }

    /* 处理整数为时0的情况 */
    if (dec == 0)
    {
        _putchar('0');
		return;
    }
    else
    {
        printNum(dec, 10); // 打印十进制数
    }
}

void printNum(unsigned long num, int base)
{
    /* 递归结束条件 */
	if (num == 0)
    {
        return;
    }
    
    /* 继续递归 */
	printNum(num/base, base);

	/* 逆序打印结果 */
    _putchar("0123456789abcdef"[num%base]);
}

void pause()
{
	int i = 0;
	int j = 0;
	for( i=0;i<10000;i++ )
		for( j=0;j<10000;j++ )
		{
            j+=1;
            j-=1;
		}
}

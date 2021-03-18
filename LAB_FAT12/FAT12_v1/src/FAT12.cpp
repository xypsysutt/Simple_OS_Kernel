#include <iostream>
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <vector>
#include <string.h>
#include <ctime>
#include <cstdio>
#include <iomanip>
using namespace std;
typedef unsigned char u8;   //1字节
typedef unsigned short u16; //2字节
typedef unsigned int u32;   //4字节
int BytsPerSec;				//每扇区字节数
int SecPerClus;				//每簇扇区数
int RsvdSecCnt;				//Boot记录占用的扇区数
int NumFATs;				//FAT表个数
int RootEntCnt;				//根目录最大文件数
int FATSz;					//FAT扇区数
int fatBase;				//Boot记录占用的扇区数*每扇区字节数=boot记录占用字节数
int fileRootBase;			//（Boot记录占用的扇区数+FAT表个数* FAT扇区数）*每扇区字节数
int dataBase;				//（Boot记录占用的扇区数+FAT表个数* FAT扇区数+（根目录最大文件数* 32+每扇区字节数-1）/每扇区字节数）*每扇区字节数
int BytsPerClus;
#pragma pack(1) /*指定按1字节对齐*/
struct BPB
{
	u16 BPB_BytsPerSec; //每扇区字节数
	u8 BPB_SecPerClus;  //每簇扇区数
	u16 BPB_RsvdSecCnt; //Boot记录占用的扇区数
	u8 BPB_NumFATs;		//FAT表个数
	u16 BPB_RootEntCnt; //根目录最大文件数
	u16 BPB_TotSec16;
	u8 BPB_Media;
	u16 BPB_FATSz16; //FAT扇区数
	u16 BPB_SecPerTrk;
	u16 BPB_NumHeads;
	u32 BPB_HiddSec;
	u32 BPB_TotSec32; //如果BPB_FATSz16为0，该值为FAT扇区数
}; // size = 25Bytes
//根目录条目
struct RootEntry
{
	char DIR_Name[11];
	u8 DIR_Attr; //文件属性
	char reserved[10];
	u16 DIR_WrtTime;
	u16 DIR_WrtDate;
	u16 DIR_FstClus; //开始簇号
	u32 DIR_FileSize;
}; // size = 32Bytes
#pragma pack() /*取消指定对齐，恢复缺省对齐*/
typedef struct RootEntry *fptr;
struct fnode
{
	char rname[40] = { 0 };
	char fname[12] = { 0 };
	RootEntry fentry;
	void append(const char *ch) { strcat(fname, ch); }
	void rappend(const char *ch) { strcat(rname, ch); }
};
int checkFile(char *, int);
void getFname(const char *);
void getRname(const char *);
unsigned short getTime();
unsigned short getDate();
void showTime(unsigned short);
void showDate(unsigned short);
// 写入生成文本文件目录项内容
RootEntry WirteIntext(FILE *fat12);
// 写入生成文本文件的目录项并返回此目录项地址
void getWritrEntry(FILE *fat12, string tname);
// 初始化根目录项
void getRootFiles(FILE *fat12, struct RootEntry *rootEntry_ptr); 
int getFATValue(FILE *fat12, int num);
void dfs(FILE *fat12, fnode cur, char *rname);
void op(FILE *fat12, string tname);
void edit(FILE *fat12, string fname);
void del(FILE *fat12, string fname);
void dir(FILE *fat12, string rname);
void print(char *, int);
void ls(FILE *fat12);
char transletter(char x);
// 从BPB表初始化FAT信息
void ini(FILE* fat12);
char dot1[10] = ".  ..  ";
char dot2[10] = ".\n..\n";
char colon[2] = ":";
char tdata[100000];
char fname_tmp[13];
// 根目录文件队列
vector<fnode> froot;
// 文件树
vector<fnode> ftree;
void print(char *x, int a) {
	x[a] = '\0';
	printf("%s", x);
}
unsigned short getTime() {
	time_t nowtime;
	struct tm* p;;
	time(&nowtime);
	p = localtime(&nowtime);
	unsigned short ans = (p->tm_hour << 11) + (p->tm_min << 5);
	return ans;
}
void showTime(unsigned short time) {
	u16 tm_min = 0b11111100000;
	u16 tm_hour = 0b1111100000000000;
	tm_min = ((tm_min&time) >> 5);
	tm_hour = ((tm_hour&time) >> 11);
	cout << setw(2) << tm_hour << ":" << setw(2) << tm_min;
}
unsigned short getDate() {
	time_t nowtime;
	struct tm* p;;
	time(&nowtime);
	p = localtime(&nowtime);
	unsigned short year = p->tm_year - 80;
	unsigned short mon = p->tm_mon + 1;
	unsigned short ans = (year << 9) + (mon << 5) + p->tm_mday;
	return ans;
}
void showDate(unsigned short date) {
	u16 year = 0b1111111000000000;
	u16 month = 0b111100000;
	u16 day = 0b11111;
	year = ((year&date) >> 9) + 1980;
	month = ((month&date) >> 5);
	day = (day & date);
	cout << year << "/" << right << setw(2) << setfill('0') << month << "/" << setw(2) << setfill('0') << day << "    ";
}
void getTextData(FILE *fat12, RootEntry re)
{
	int curClus = re.DIR_FstClus;
	int startByte;
	int fsize = re.DIR_FileSize;
	while (curClus < 0xFF8)
	{
		if (curClus == 0xFF7)
		{
			
			printf("bad cluster,read failed\n");
			break;
		}
		startByte = dataBase + (curClus - 2) * BytsPerClus;
		fseek(fat12, startByte, SEEK_SET);
		if (fsize >= BytsPerClus) {
			fread(tdata, 1, BytsPerClus, fat12);
			fsize -= BytsPerClus;
		}
		else
			fread(tdata, 1, fsize, fat12);
		tdata[fsize - 1] = '\0';
		cout << tdata;
		curClus = getFATValue(fat12, curClus); //获取fat项的内容
	}
}
void getTextEntry(FILE *fat12, RootEntry re, string tname)
{
	int curClus = re.DIR_FstClus;
	int startByte;
	while (curClus < 0xFF8)
	{
		if (curClus == 0xFF7)
		{
			
			printf("bad cluster,read failed\n");
			break;
		}
		startByte = dataBase + (curClus - 2) * BytsPerClus;
		for (int loop = 0; loop < BytsPerClus; loop += 32)
		{
			RootEntry roottmp;
			fptr rootptr = &roottmp;
			fseek(fat12, startByte + loop, SEEK_SET);
			fread(rootptr, 1, 32, fat12);
			if ((rootptr->DIR_Name[0] == '\0') || (checkFile(rootptr->DIR_Name, 0) == 0) || (rootptr->DIR_Attr & 0x10) != 0)
				continue;
			getFname(rootptr->DIR_Name);
			string tmp = fname_tmp;
			if (tmp == tname)
			{
				getTextData(fat12, *rootptr);
				break;
			}
		}
		curClus = getFATValue(fat12, curClus); //获取fat项的内容
	}
}
int getFATValue(FILE *fat12, int num) //3072
{
	u16 bytes;
	u16 *bytes_ptr = &bytes;
	fseek(fat12, fatBase + num * 3 / 2, SEEK_SET);
	fread(bytes_ptr, 1, 2, fat12);	
	return (num & 1) ? (bytes >> 4) : (bytes & ((1 << 12) - 1)); 
}
int checkFile(char *fname_tmp, int pos)
{
	for (int j = pos; j < pos + 11; j++)
	{
		if (!(((fname_tmp[j] >= 48) && (fname_tmp[j] <= 57)) ||
			((fname_tmp[j] >= 65) && (fname_tmp[j] <= 90)) ||
			((fname_tmp[j] >= 97) && (fname_tmp[j] <= 122)) ||
			(fname_tmp[j] == ' ')))
			return 0;
	}
	return 1;
}
void getRname(const char *dirname)
{
	int tmplen = 0;
	for (int k = 0; k < 12 && dirname[k] != ' '; k++)
		fname_tmp[tmplen++] = transletter(dirname[k]);
	fname_tmp[tmplen] = '\0';
}
void getFname(const char *dirname)
{
	int tmplen = 0;
	for (int k = 0; k < 11; k++)
	{
		if (dirname[k] != ' ')
			fname_tmp[tmplen++] = transletter(dirname[k]);
		else
		{
			fname_tmp[tmplen++] = '.';
			while ((dirname[k] == ' ') && k < 11) // 过滤空格
				k++;
			k--;
		}
	}
	fname_tmp[tmplen] = '\0';
}
char transletter(char x)
{
	if ((x <= 'z') && (x >= 'a'))
	{
		x -= 32;
	}
	return x;
}
void help() {
	cout << "                                        -ls 展开文件树" << endl;
	cout << "                                        -dir 展开文件详情" << endl;
	cout << "                                        -creat 创建文本文件" << endl;
	cout << "                                        -edit 编辑文本文件" << endl;
	cout << "                                        -del 删除文件" << endl;
	cout << "                                        -op 打开文件" << endl;
	cout << "                                        -q 退出" << endl;
}
int main() {
	FILE *fat12;
	fat12 = fopen("a.img", "r+");
	ini(fat12);
	help();
	printf(">");
	string cmd, tname;
	cin >> cmd ;
	while (1) {
		if (cmd == "q" || cmd == "Q") {
			cout << "Quit." << endl;
			return 0;
		}
		else if (cmd == "creat") {
			cin >> tname;
			froot.clear();
			ftree.clear();
			ini(fat12);
			getWritrEntry(fat12, tname);
			cin.clear();
			printf("\n>");
			cin >> cmd;
		}
		else if (cmd == "ls") {
			froot.clear();
			ftree.clear();
			ini(fat12);
			ls(fat12);
			printf("\n>");
			cin >> cmd;
		}
		else if (cmd == "dir") {
			cin >> tname;
			froot.clear();
			ftree.clear();
			ini(fat12);
			dir(fat12, tname);
			printf("\n>");
			cin >> cmd;
		}
		else if (cmd == "del") {
			cin >> tname;
			froot.clear();
			ftree.clear();
			ini(fat12);
			del(fat12, tname);
			printf("\n>");
			cin >> cmd;
		}
		else if (cmd == "edit") {
			cin >> tname;
			froot.clear();
			ftree.clear();
			ini(fat12);
			edit(fat12, tname);
			printf("\n>");
			cin >> cmd;
		}
		else if (cmd == "op") {
			cin >> tname;
			froot.clear();
			ftree.clear();
			ini(fat12);
			op(fat12, tname);
			printf("\n>");
			cin >> cmd;
		}
		else {
			cin.clear();
			printf("\n>");
			cin >> cmd;
		}
	}
	return 0;
}

void ini(FILE *fat12)
{
	struct BPB bpb;
	struct BPB *bpb_ptr = &bpb;   //载入BPB
	fseek(fat12, 11, SEEK_SET);   //BPB从偏移11个字节处开始
	fread(bpb_ptr, 1, 25, fat12); //BPB长度为25字节

	BytsPerSec = bpb_ptr->BPB_BytsPerSec; //初始化各个全局变量
	SecPerClus = bpb_ptr->BPB_SecPerClus;
	RsvdSecCnt = bpb_ptr->BPB_RsvdSecCnt;
	NumFATs = bpb_ptr->BPB_NumFATs;
	RootEntCnt = bpb_ptr->BPB_RootEntCnt;
	if (bpb_ptr->BPB_FATSz16 != 0)
		FATSz = bpb_ptr->BPB_FATSz16;
	else
		FATSz = bpb_ptr->BPB_TotSec32;
	fatBase = RsvdSecCnt * BytsPerSec;
	fileRootBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec; //根目录首字节的偏移数=boot+fat1&2的总字节数
	dataBase = BytsPerSec * (RsvdSecCnt + FATSz * NumFATs + (RootEntCnt * 32 + BytsPerSec - 1) / BytsPerSec);
	BytsPerClus = SecPerClus * BytsPerSec; //每簇的字节数
	struct RootEntry rootEntry;
	fptr rootEntry_ptr = &rootEntry;
	getRootFiles(fat12, rootEntry_ptr);
	for (auto ele : froot) {
		if ((ele.fentry.DIR_Attr & 0x10) == 0)
			continue;
		char tmp[50];
		memset(tmp, 0, sizeof(tmp));
		strcat(tmp, ele.rname);
		strcat(tmp, ele.fname);
		ftree.push_back(ele);
		dfs(fat12, ele, tmp);
	}
}
void getRootFiles(FILE *fat12, fptr rootEntry_ptr) {
	int frbase = fileRootBase;
	for (int i = 0; i < RootEntCnt; i++)
	{
		fseek(fat12, frbase, SEEK_SET);
		fread(rootEntry_ptr, 1, 32, fat12);
		frbase += 32;
		if ((rootEntry_ptr->DIR_Name[0] == '\0') || (checkFile(rootEntry_ptr->DIR_Name, 0) == 0)) //过滤非法条目
			continue;
		fnode f;
		if ((rootEntry_ptr->DIR_Attr & 0x10) == 0) //此条目是文件
		{
			getFname(rootEntry_ptr->DIR_Name);
		}
		else //目录   则放进队列
		{
			getRname(rootEntry_ptr->DIR_Name);
			f.rappend("/");
		}
		f.append(fname_tmp);
		f.fentry = *rootEntry_ptr;
		froot.push_back(f);
	}
}
void dfs(FILE *fat12, fnode cur, char *rname)
{
	int curClus = cur.fentry.DIR_FstClus;
	int startByte;
	while (curClus < 0xFF8)
	{
		if (curClus == 0xFF7)
		{
			printf("Bad cluster\n");
			break;
		}
		startByte = dataBase + (curClus - 2) * BytsPerClus;
		for (int loop = 0; loop < BytsPerClus; loop += 32)
		{
			RootEntry roottmp;
			fptr rootptr = &roottmp;
			fseek(fat12, startByte + loop, SEEK_SET);
			fread(rootptr, 1, 32, fat12);
			if ((rootptr->DIR_Name[0] == '\0') || (checkFile(rootptr->DIR_Name, 0) == 0) || (rootptr->DIR_Attr & 0x10) == 0)
				continue;
			getRname(rootptr->DIR_Name);
			fnode f;
			f.append(fname_tmp);
			f.rappend(rname);
			f.rappend("/");
			f.fentry = *rootptr;
			char tmp[50];
			memset(tmp, 0, sizeof(tmp));
			strcat(tmp, f.rname);
			strcat(tmp, f.fname);
			ftree.push_back(f);
			dfs(fat12, f, tmp);
		}
		curClus = getFATValue(fat12, curClus); //获取fat项的内容
	}
}
/*
 * 指令格式：creat /文件路径/文件名 （es. /USER/HOULAI.TXT；ps.根目录下为/HOULAI.TXT）
 * 指令功能：生成文本文件，并可编辑内容
 */
RootEntry WirteIntext(FILE *fat12) {
	int curClus;
	int startByte;
	RootEntry tpEntry;
	//fptr tpEntryptr = (fptr)malloc(sizeof(RootEntry));
	string tpLine;
	int fsize = 0;
	vector<string> tpText;
	getchar();
	while (getline(cin, tpLine)) {
		tpText.push_back(tpLine);
		fsize += tpLine.length();
		fsize++; // \n
	}
	tpEntry.DIR_FileSize = fsize;
	for (int i = 0; i < 3072; i++) {
		if (getFATValue(fat12, i) == 0) {
			int startByte = dataBase + (i - 2) * BytsPerClus;
			fseek(fat12, startByte, SEEK_SET);
			for (auto ele : tpText) {
				char* tp = (char*)malloc(ele.length() + 1);
				strcpy(tp, ele.c_str());
				fwrite(tp, strlen(tp), 1, fat12);
				free(tp);
				fwrite("\n", 1, 1, fat12);
			}

			tpEntry.DIR_FstClus = (short)i;
			tpEntry.DIR_WrtDate = (unsigned short)getDate();
			tpEntry.DIR_WrtTime = (unsigned short)getTime();
			tpEntry.DIR_Attr = (char)0;

			u16 bytes;
			u16 bytes2 = 0xff8;
			u16 *bytes_ptr = &bytes;
			fseek(fat12, fatBase + i * 3 / 2, SEEK_SET);
			fread(bytes_ptr, 1, 2, fat12);
			if (i & 1) {
				bytes2 = (bytes2 << 4) + (bytes & 15);
			}
			else {
				bytes2 = bytes2 + (bytes&(61440));
			}
			fseek(fat12, fatBase + i * 3 / 2, SEEK_SET);
			fwrite(&bytes2, sizeof(u16), 1, fat12);
			fseek(fat12, fatBase + i * 3 / 2 + FATSz * 512, SEEK_SET);
			fwrite(&bytes2, sizeof(u16), 1, fat12);
			//free(tpEntryptr);
			return tpEntry;
		}
	}
}
void getWritrEntry(FILE *fat12, string tname) {
	RootEntry newEntry;
	RootEntry tpEntry = WirteIntext(fat12);
	fptr newEptr = &newEntry;
	string sroot, stext;
	newEptr->DIR_Attr = 1;
	int pos = tname.rfind('/');//  /name.txt
	if (pos < 0) {
		printf("Root is not find,please retype it\n");
		return;
	}
	if (pos == 0) {
		sroot = "/";
		stext = tname.substr(pos + 1);
		int frbase = fileRootBase;
		fptr rootEntry_ptr = &newEntry;
		for (int i = 0; i < RootEntCnt; i++) {
			fseek(fat12, frbase, SEEK_SET);
			fread(rootEntry_ptr, 1, 32, fat12);
			frbase += 32;
			if ((rootEntry_ptr->DIR_Name[0] != '\0')) //过滤非法条目
				continue;
			pos = stext.rfind('.');
			string prefix, postfix;
			if (pos >= 8) {
				prefix = stext.substr(0, 8);
				postfix = stext.substr(pos + 1).substr(0, 3);
				stext = prefix + postfix;
			}
			else {
				prefix = stext.substr(0, pos);
				postfix = stext.substr(pos + 1).substr(0, 3);
				for (int i = 0; i <= 7 - pos; i++) {
					prefix += ' ';
				}
				stext = prefix + postfix;
			}
			strcpy(rootEntry_ptr->DIR_Name, stext.c_str());
			rootEntry_ptr->DIR_Attr = 0x20;
			rootEntry_ptr->DIR_FileSize = tpEntry.DIR_FileSize;
			rootEntry_ptr->DIR_FstClus = tpEntry.DIR_FstClus;
			rootEntry_ptr->DIR_WrtDate = tpEntry.DIR_WrtDate;
			rootEntry_ptr->DIR_WrtTime = tpEntry.DIR_WrtTime;
			fseek(fat12, -32, SEEK_CUR);
			fwrite(rootEntry_ptr, sizeof(RootEntry), 1, fat12);
			return;
		}

	}
	else {
		sroot = tname.substr(0, pos);
		stext = tname.substr(pos + 1);
		for (auto ele : ftree) {
			string x = ele.rname;
			x += ele.fname;
			if (x == sroot) {
				fptr rootEntry_ptr = &newEntry;
				int startByte = dataBase + (ele.fentry.DIR_FstClus - 2) * BytsPerClus;
				fseek(fat12, startByte, SEEK_SET);
				fread(rootEntry_ptr, 1, 32, fat12);
				while (rootEntry_ptr->DIR_Name[0] != '\0') {
					startByte += 32;
					fseek(fat12, startByte, SEEK_SET);
					fread(rootEntry_ptr, 1, 32, fat12);
				}
				pos = stext.rfind('.');
				string prefix, postfix;
				if (pos >= 8) {
					prefix = stext.substr(0, 8);
					postfix = stext.substr(pos + 1).substr(0, 3);
					stext = prefix + postfix;
				}
				else {
					prefix = stext.substr(0, pos);
					postfix = stext.substr(pos + 1).substr(0, 3);
					for (int i = 0; i <= 7 - pos; i++) {
						prefix += ' ';
					}
					stext = prefix + postfix;
				}
				strcpy(rootEntry_ptr->DIR_Name, stext.c_str());
				rootEntry_ptr->DIR_Attr = 0x01;
				rootEntry_ptr->DIR_FileSize = tpEntry.DIR_FileSize;
				rootEntry_ptr->DIR_FstClus = tpEntry.DIR_FstClus;
				rootEntry_ptr->DIR_WrtDate = tpEntry.DIR_WrtDate;
				rootEntry_ptr->DIR_WrtTime = tpEntry.DIR_WrtTime;
				fseek(fat12, startByte, SEEK_SET);
				fwrite(rootEntry_ptr, sizeof(RootEntry), 1, fat12);
			}
		}
	}
	return;
}
/*
 * 指令格式： ls
 * 指令功能： 展开文件树
 */
struct lsfile
{
	bool flag;
	char fname[12];
	void append(char *ch) { strcat(fname, ch); }
	void ini()
	{
		memset(fname, 0, sizeof(fname));
	}
	int fsize;
};
vector<lsfile> vcfile;
void Traverse(fnode cur, FILE *fat12)
{
	vcfile.clear();
	int curClus = cur.fentry.DIR_FstClus, startByte;
	while (curClus < 0xFF8)
	{
		if (curClus == 0xFF7)
		{
			printf("bad cluster,read failed\n");
			break;
		}
		startByte = dataBase + (curClus - 2) * BytsPerClus;
		for (int loop = 0; loop < BytsPerClus; loop += 32)
		{
			RootEntry roottmp;
			fptr rootptr = &roottmp;
			fseek(fat12, startByte + loop, SEEK_SET);
			fread(rootptr, 1, 32, fat12);
			if ((rootptr->DIR_Name[0] == '\0') || (checkFile(rootptr->DIR_Name, 0) == 0))
				continue;
			if ((rootptr->DIR_Attr & 0x10) == 0)
			{
				lsfile tpfile;
				tpfile.ini();
				tpfile.flag = 0;
				getFname(rootptr->DIR_Name);
				tpfile.append(fname_tmp);
				tpfile.fsize = rootptr->DIR_FileSize;
				vcfile.push_back(tpfile);
			}
			else
			{
				lsfile tpfile;
				tpfile.ini();
				tpfile.flag = 1;
				getRname(rootptr->DIR_Name);
				tpfile.append(fname_tmp);
				vcfile.push_back(tpfile);
			}
		}
		curClus = getFATValue(fat12, curClus);
	}
	return;
}
void ls(FILE *fat12) {
	int mode;
	printf("/:");
	for (auto ele : froot)
	{
		print(ele.fname,strlen(ele.fname));
		printf(" ");
	}
	printf("\n");

	for (auto ele : ftree)
	{
		Traverse(ele, fat12);
		print(ele.rname, strlen(ele.rname));
		print(ele.fname, strlen(ele.fname));
		printf("/:");
		printf("\n");
		printf("%s", dot1);
		for (auto obj : vcfile)
		{
			print(obj.fname, strlen(obj.fname));
			printf(" ");
		}
		printf("\n");
	}
}
/*
 * 指令格式：dir /文件路径 （es. /USER/；ps.根目录下为/）
 * 指令功能：展示指定路径下文件项
 */
void dir(FILE *fat12, string rname) {
	int flag = 0;
	RootEntry newEntry;
	fptr rootEntry_ptr = &newEntry;
	if (rname == "/") {
		cout << left << setw(20) <<setfill(' ')<< "name" << right << setw(15) << "size" << setw(39) << "date" << endl;
		for (auto ele : froot) {
			rootEntry_ptr = &ele.fentry;
			if (rootEntry_ptr->DIR_Name[0] != '\0') {
				memset(fname_tmp, ' ', sizeof(fname_tmp));
				if ((rootEntry_ptr->DIR_Attr & 0x10) == 0)
					getFname(rootEntry_ptr->DIR_Name);
				else
					getRname(rootEntry_ptr->DIR_Name);
				print(fname_tmp, strlen(fname_tmp));
				if (strlen(fname_tmp) < 13) {
					for (int i = 0; i < 13 - strlen(fname_tmp); i++)
						printf(" ");
				}
				cout << setfill(' ') << left << setw(20) << "" << setw(6) << rootEntry_ptr->DIR_FileSize << setw(25) << " ";
				showDate(rootEntry_ptr->DIR_WrtDate);
				showTime(rootEntry_ptr->DIR_WrtTime);
				cout << endl;
			}
		}
	}
	else
		for (auto ele : ftree) {
			string x = ele.rname;
			x += ele.fname;
			if (x == rname) {
				flag = 1;
				int startByte = dataBase + (ele.fentry.DIR_FstClus - 2) * BytsPerClus + 64;
				fseek(fat12, startByte, SEEK_SET);
				fread(rootEntry_ptr, 1, 32, fat12);
				cout << left << setw(20) << "name" << right << setw(15) << "size" << setw(39) << "date" << endl;
				while (rootEntry_ptr->DIR_Name[0] != '\0') {
					memset(fname_tmp, ' ', sizeof(fname_tmp));
					if ((rootEntry_ptr->DIR_Attr & 0x10) == 0)
						getFname(rootEntry_ptr->DIR_Name);
					else
						getRname(rootEntry_ptr->DIR_Name);
					print(fname_tmp, strlen(fname_tmp));
					if (strlen(fname_tmp) < 13) {
						for (int i = 0; i < 13 - strlen(fname_tmp); i++)
							printf(" ");
					}
					cout << setfill(' ') << left << setw(20) << "" << setw(6) << rootEntry_ptr->DIR_FileSize << setw(25) << " ";
					showDate(rootEntry_ptr->DIR_WrtDate);
					showTime(rootEntry_ptr->DIR_WrtTime);
					cout << endl;
					startByte += 32;
					fseek(fat12, startByte, SEEK_SET);
					fread(rootEntry_ptr, 1, 32, fat12);
				}
			}
		}
	if (flag = 0) {
		cout << "The root rount is not found, please retype it.\n";
	}
}
/*
 * 指令格式：del /文件路径/文件名 （es. /USER/HOULAI.TXT；ps.根目录下为/HOULAI.TXT）
 * 指令功能：删除指定文件
 */
void del(FILE *fat12, string fname) {
	RootEntry newEntry;
	fptr newEptr = &newEntry;
	string sroot, stext;
	newEptr->DIR_Attr = 1;
	int pos = fname.rfind('/');//  /name.txt
	if (pos < 0) {
		printf("The root rount is not found, please retype it.\n");
		return;
	}
	if (pos == 0) {
		sroot = "/";
		stext = fname.substr(pos + 1);
		//stext = sroot + stext;
		int frbase = fileRootBase;
		fptr rootEntry_ptr = &newEntry;
		for (int i = 0; i < RootEntCnt; i++) {
			fseek(fat12, frbase, SEEK_SET);
			fread(rootEntry_ptr, 1, 32, fat12);
			frbase += 32;
			memset(fname_tmp, ' ', sizeof(fname_tmp));
			if ((rootEntry_ptr->DIR_Attr & 0x10) == 0)
				getFname(rootEntry_ptr->DIR_Name);
			else
				getRname(rootEntry_ptr->DIR_Name);
			string nn = fname_tmp;
			nn = nn.substr(0, fname.size() - 1);
			if (nn == stext) {
				int clus_num = rootEntry_ptr->DIR_FstClus;
				fseek(fat12, fatBase + clus_num * 3 / 2, SEEK_SET);
				unsigned short w = 0;
				unsigned short* pw = &w;
				fwrite(pw, sizeof(u16), 1, fat12);
				fseek(fat12, fatBase + clus_num * 3 / 2 + FATSz * 512, SEEK_SET);
				fwrite(pw, sizeof(u16), 1, fat12);
				fseek(fat12, frbase - 32, SEEK_SET);
				char n = '\0';
				char *np = &n;
				for (int i = 0; i < 32; i++)
					fwrite(np, 1, 1, fat12);

			}

		}

	}
	else {
		sroot = fname.substr(0, pos);
		stext = fname.substr(pos + 1);
		for (auto ele : ftree) {
			string x = ele.rname;
			x += ele.fname;
			if (x == sroot) {
				fptr rootEntry_ptr = &newEntry;
				int startByte = dataBase + (ele.fentry.DIR_FstClus - 2) * BytsPerClus;
				fseek(fat12, startByte, SEEK_SET);
				fread(rootEntry_ptr, 1, 32, fat12);
				memset(fname_tmp, ' ', sizeof(fname_tmp));
				if ((rootEntry_ptr->DIR_Attr & 0x10) == 0)
					getFname(rootEntry_ptr->DIR_Name);
				else
					getRname(rootEntry_ptr->DIR_Name);
				string nn = fname_tmp;
				nn = nn.substr(0, fname.size());
				if (strcmp(nn.c_str(), stext.c_str()) == 0) {
					int clus_num = rootEntry_ptr->DIR_FstClus;
					fseek(fat12, fatBase + clus_num * 3 / 2, SEEK_SET);
					fwrite(0, 1, sizeof(u16), fat12);
					fseek(fat12, fatBase + clus_num * 3 / 2 + FATSz * 512, SEEK_SET);
					fwrite(0, 1, sizeof(u16), fat12);
					fseek(fat12, startByte, SEEK_SET);
					fwrite(0, 1, 32, SEEK_SET);
				}
			}
		}
	}
	return;
}
/*
 * 指令格式：edit /文件路径/文件名 （es. /USER/HOULAI.TXT；ps.根目录下为/HOULAI.TXT）
 * 指令功能：编辑指定文件内容或重命名
 */
void edit(FILE *fat12, string fname) {
	RootEntry newEntry;
	fptr newEptr = &newEntry;
	string sroot, stext;
	newEptr->DIR_Attr = 1;
	RootEntry tpEntry;
	bool flag = 0;
	string tpLine;
	vector<string> tpText;
	int fsize = 0;
	int pos = fname.rfind('/');//  /name.txt
	/*if (pos < 0) {
		printf("%s", warn5);
		return;
	}*/
	if (pos <= 0) {
		int frbase = fileRootBase;
		fptr rootEntry_ptr = &newEntry;
		for (int i = 0; i < RootEntCnt; i++) {
			fseek(fat12, frbase, SEEK_SET);
			fread(rootEntry_ptr, 1, 32, fat12);
			frbase += 32;
			memset(fname_tmp, ' ', sizeof(fname_tmp));
			if ((rootEntry_ptr->DIR_Attr & 0x10) == 0)
				getFname(rootEntry_ptr->DIR_Name);
			else
				getRname(rootEntry_ptr->DIR_Name);
			string nn = fname_tmp;
			nn = nn.substr(0, fname.size());
			if (strcmp(nn.c_str(), fname.c_str()) == 0) {
				int clus_num = rootEntry_ptr->DIR_FstClus;
				getchar();
				while (getline(cin, tpLine)) {
					tpText.push_back(tpLine);
					fsize += tpLine.length();
					fsize++; // \n
				}
				int startByte = dataBase + (clus_num - 2) * BytsPerClus;
				fseek(fat12, startByte, SEEK_SET);
				for (auto ele : tpText) {
					char* tp = (char*)malloc(ele.length() + 1);
					strcpy(tp, ele.c_str());
					fwrite(tp, strlen(tp), 1, fat12);
					free(tp);
					fwrite("\n", 1, 1, fat12);
				}
				fseek(fat12, frbase - 32, SEEK_SET);
				rootEntry_ptr->DIR_WrtDate = getDate();
				rootEntry_ptr->DIR_WrtTime = getTime();
				printf("Would you rename the file ? (EOF to giv up)\n");
				printf("Enter the new name:\n");
				//getchar();
				cin.clear();
				if (getline(cin, tpLine)) {
					string stext = tpLine;
					int pos = stext.rfind('.');//未做鲁棒性处理
					string prefix, postfix;
					if (pos >= 8) {
						prefix = stext.substr(0, 8);
						postfix = stext.substr(pos + 1).substr(0, 3);
						stext = prefix + postfix;
					}
					else {
						prefix = stext.substr(0, pos);
						postfix = stext.substr(pos + 1).substr(0, 3);
						for (int i = 0; i <= 7 - pos; i++) {
							prefix += ' ';
						}
						stext = prefix + postfix;
					}
					strcpy(rootEntry_ptr->DIR_Name, stext.c_str());

				}
				cin.clear();
				rootEntry_ptr->DIR_FileSize = fsize;
				fwrite(rootEntry_ptr, sizeof(RootEntry), 1, fat12);
				return;
			}

		}

	}
	else {
		printf("The file is not found, please retype it.\n");
	}
	return;
}
/*
 * 指令格式：op /文件路径/文件名 （es. /USER/HOULAI.TXT；ps.根目录下为/HOULAI.TXT）
 * 指令功能：打开文本文件
 */
void op(FILE *fat12, string tname)
{
	string str = tname, sroot, stext;
	bool flag = 0;
	int pos = str.rfind('/');
	if (pos < 0)
	{
		stext = str;
		for (auto ele : froot)
		{
			if (strcmp(ele.fname, tname.c_str()) == 0)
			{
				getTextData(fat12, ele.fentry);
				flag = 1;
				break;
			}
		}
	}
	sroot = str.substr(0, pos);
	stext = str.substr(pos + 1);
	for (auto ele : ftree)
	{
		string x = ele.rname;
		x += ele.fname;
		if (x == sroot)
		{
			getTextEntry(fat12, ele.fentry, stext);
			flag = 1;
			break;
		}
	}
	if (flag == 0)
	{
		printf("The file is not found, please retype it.\n");
	}
}

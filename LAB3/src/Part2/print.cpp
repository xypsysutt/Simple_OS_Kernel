#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" char* my_str();
extern "C" void* my_print(char *);
int main(){
    char *ptr = my_str();
    int num[26]={};
    char tp[50]={};
    printf("%s\n",ptr);
    for(int i=0;i<strlen(ptr);i++){
        num[ptr[i]-'a']++;
    }
    for(int i=0;i<26;i++){
        if(num[i]>0){
            tp[0]=i+'a';
            tp[1]=':';
            tp[2]=num[i]+'0';
            tp[3]='\n';
            my_print(tp);
        }
    }

}
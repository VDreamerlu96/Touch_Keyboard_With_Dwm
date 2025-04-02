#include<stdlib.h>
#include<stdio.h>

void Error_Exit(const char* einfo,void (*clean_function)(int)){
    printf("\nEinfo: %s",einfo);
    if(clean_function){
        exit(0);
    }else{
        clean_function(0);
    }
    exit(0);
}
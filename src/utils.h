#pragma once
#include <stdlib.h>
#include <stdio.h>

static inline char* loadKernel(const char* filename) {
    FILE* f = fopen(filename,"rb");
    if(!f){ perror("open kernel"); exit(1);}
    fseek(f,0,SEEK_END);
    size_t size = ftell(f);
    fseek(f,0,SEEK_SET);
    char* src=(char*)malloc(size+1);
    fread(src,1,size,f);
    src[size]='\0';
    fclose(f);
    return src;
}

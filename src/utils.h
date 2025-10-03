#pragma once
#include <stdlib.h>
#include <stdio.h>

static inline const char* loadKernel(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if(!f) { printf("Cannot open kernel file.\n"); return NULL; }
    fseek(f,0,SEEK_END);
    size_t size = ftell(f);
    fseek(f,0,SEEK_SET);
    char* src = (char*)malloc(size+1);
    fread(src,1,size,f);
    src[size] = '\0';
    fclose(f);
    return src;
}

static inline void loadOBJ(const char* filename)
{

}

#pragma once

#include <CL/cl.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
  cl_platform_id platform;
  cl_device_id device;
  cl_program program;
  cl_context context;
  cl_command_queue queue;
  cl_int err;
} CL;

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

inline CL clInit(const char* kernel)
{
  CL cl = {0};

  clGetPlatformIDs(1, &cl.platform, NULL);
  clGetDeviceIDs(cl.platform, CL_DEVICE_TYPE_GPU, 1, &cl.device, NULL);

  cl.context = clCreateContext(NULL, 1, &cl.device, NULL, NULL, NULL);
  cl.queue = clCreateCommandQueue(cl.context, cl.device, 0, NULL);
  

  const char* kernelSource = loadKernel(kernel); 
  cl.program = clCreateProgramWithSource(cl.context, 1, &kernelSource, NULL, &cl.err);
  if (cl.err != CL_SUCCESS) { printf("Error creating program: %d\n", cl.err); }

  cl.err = clBuildProgram(cl.program, 1, &cl.device, NULL, NULL, NULL);
  if (cl.err != CL_SUCCESS) {
      size_t log_size;
      clGetProgramBuildInfo(cl.program, cl.device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
      char* log = (char*)malloc(log_size);
      clGetProgramBuildInfo(cl.program, cl.device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
      printf("OpenCL build error:\n%s\n", log);
      free(log);
  }
  return cl;
}

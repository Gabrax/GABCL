#include <windows.h>
#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "window.h"

#define WIDTH 800
#define HEIGHT 600
#define NUM_SHAPES 3

#define SHAPE_SQUARE   0
#define SHAPE_CIRCLE   1
#define SHAPE_TRIANGLE 2

typedef struct {
    int type;           
    float x, y;          
    float size;          
    float angle;
    unsigned int color; 
} Shape;

void shape_translate(Shape* shape, float x, float y, float deltaTime)
{
    shape->x += x * deltaTime;
    shape->y += y * deltaTime;
}

void shape_rotate(Shape* shape, float deltaRadians)
{
  shape->angle += deltaRadians;
  if (shape->angle > 2.0f*M_PI) shape->angle -= 2.0f*M_PI;
  if (shape->angle < 0.0f) shape->angle += 2.0f*M_PI;
}

void shape_scale(Shape* shape, float scale)
{
  float oldSize = shape->size;
  shape->size *= scale;
  shape->x -= (shape->size - oldSize)/2;
  shape->y -= (shape->size - oldSize)/2;
}

Shape shapes[NUM_SHAPES] = {
    { SHAPE_SQUARE, 100, 100, 50, 0, 0xFFFF0000u},
    { SHAPE_CIRCLE, 200, 100, 50, 0, 0xFF00FF00u},
    { SHAPE_TRIANGLE, 300, 100, 50, 0, 0xFF0000FFu}
};

char* loadKernel(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) { perror("Failed to open kernel file"); exit(1); }

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* src = (char*)malloc(size + 1);
    fread(src, 1, size, f);
    src[size] = '\0';
    fclose(f);

    return src;
}

int main()
{
    Window window;
    window_init(&window, "GABCL", WIDTH, HEIGHT);

    cl_int err;
    cl_platform_id platform = NULL;
    cl_device_id device = NULL;
    if (clGetPlatformIDs(1, &platform, NULL) != CL_SUCCESS) {
        printf("No OpenCL platform found\n"); return 1;
    }
    if (clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL) != CL_SUCCESS) {
        if (clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL) != CL_SUCCESS) {
            printf("No OpenCL device found\n"); return 1;
        } 
    }

    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err);

    char* kernelSrc = loadKernel("src/shapes.cl");
    cl_program program = clCreateProgramWithSource(context, 1, (const char**)&kernelSrc, NULL, &err);
    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    free(kernelSrc);
    if (err != CL_SUCCESS) {
        size_t logSize = 0;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
        char* log = (char*)malloc(logSize+1);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, log, NULL);
        log[logSize] = '\0';
        printf("Build log:\n%s\n", log);
        free(log);
        return 1;
    }

    cl_kernel kernelClear = clCreateKernel(program, "clear_buffer", &err);
    cl_kernel kernelDraw = clCreateKernel(program, "draw_shapes", &err);

    unsigned int* pixels = (unsigned int*)malloc(sizeof(unsigned int) * WIDTH * HEIGHT);
    cl_mem pixelBuf = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(unsigned int) * WIDTH * HEIGHT, NULL, &err);
    cl_mem shapeBuf = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(Shape) * NUM_SHAPES, NULL, &err);

    int widthVal = WIDTH;
    int heightVal = HEIGHT;
    int numShapesVal = NUM_SHAPES;

    clSetKernelArg(kernelDraw, 0, sizeof(cl_mem), &pixelBuf);
    clSetKernelArg(kernelDraw, 1, sizeof(cl_mem), &shapeBuf);
    clSetKernelArg(kernelDraw, 2, sizeof(int), &numShapesVal);
    clSetKernelArg(kernelDraw, 3, sizeof(int), &widthVal);
    clSetKernelArg(kernelDraw, 4, sizeof(int), &heightVal);

    MSG msg;
    size_t global[2] = { (size_t)WIDTH, (size_t)HEIGHT };
    
    while (1) {
      while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
          if (msg.message == WM_QUIT) goto cleanup;
          TranslateMessage(&msg);
          DispatchMessage(&msg);
      }

      float deltaTime = 1.0f / 60.0f; 

      if (GetAsyncKeyState(VK_LEFT) & 0x8000) shape_translate(&shapes[0], -20.0f, 0.0f, deltaTime);
      if (GetAsyncKeyState(VK_RIGHT) & 0x8000) shape_translate(&shapes[0], 20.0f, 0.0f, deltaTime); 
      if (GetAsyncKeyState(VK_UP) & 0x8000) shape_translate(&shapes[0], 0.0f, -20.0f, deltaTime);
      if (GetAsyncKeyState(VK_DOWN) & 0x8000) shape_translate(&shapes[0], 0.0f, 20.0f, deltaTime);
      if (GetAsyncKeyState('Q') & 0x8000) shape_rotate(&shapes[0], -0.05f * deltaTime);
      if (GetAsyncKeyState('E') & 0x8000) shape_rotate(&shapes[0], 0.05f  * deltaTime);

      clEnqueueWriteBuffer(queue, shapeBuf, CL_TRUE, 0, sizeof(Shape) * NUM_SHAPES, shapes, 0, NULL, NULL);

      clEnqueueNDRangeKernel(queue, kernelDraw, 2, NULL, global, NULL, 0, NULL, NULL);
      clFinish(queue);

      clEnqueueReadBuffer(queue, pixelBuf, CL_TRUE, 0,sizeof(unsigned int) * WIDTH * HEIGHT, pixels, 0, NULL, NULL);

      window_render(&window, pixels);
      /*Sleep(16); // ~60 FPS*/
  }


cleanup:
    clReleaseMemObject(pixelBuf);
    clReleaseMemObject(shapeBuf);
    clReleaseKernel(kernelClear);
    clReleaseKernel(kernelDraw);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    free(pixels);
    return 0;
}

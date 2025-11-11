#pragma once

#include "shapes.h"
#include "camera.h"
#include <CL/cl.h>
#include <stdlib.h>
#include <stdio.h>
#include <raylib.h>

typedef struct {
  cl_platform_id platform;
  cl_device_id device;
  cl_program program;
  cl_context context;
  cl_command_queue queue;
  cl_int err;

  cl_kernel clearKernel;
  cl_kernel vertexKernel;
  cl_kernel fragmentKernel;

  cl_mem frameBuffer;
  cl_mem depthBuffer;
  cl_mem trisBuffer;
  cl_mem mvpVertsBuffer;
  cl_mem fragPosBuffer;
  cl_mem projectionBuffer;
  cl_mem viewBuffer;
  cl_mem transformBuffer;
  cl_mem cameraPosBuffer;
  cl_mem textureBuffer;

  Color clearColor;
  size_t screen_resolution[2];
  Color* pixelBuffer;
  Texture2D texture;
} Engine;

inline const char* loadKernel(const char* filename) {
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

inline void engine_init(Engine* engine,CustomCamera* camera,CustomModel* model,const char* kernel,int width, int height)
{
  clGetPlatformIDs(1, &engine->platform, NULL);
  clGetDeviceIDs(engine->platform, CL_DEVICE_TYPE_GPU, 1, &engine->device, NULL);

  engine->context = clCreateContext(NULL, 1, &engine->device, NULL, NULL, NULL);
  engine->queue = clCreateCommandQueue(engine->context, engine->device, 0, NULL);
  
  const char* kernelSource = loadKernel(kernel); 
  engine->program = clCreateProgramWithSource(engine->context, 1, &kernelSource, NULL, &engine->err);
  if (engine->err != CL_SUCCESS) { printf("Error creating program: %d\n", engine->err); }

  engine->err = clBuildProgram(engine->program, 1, &engine->device, NULL, NULL, NULL);
  if (engine->err != CL_SUCCESS) {
      size_t log_size;
      clGetProgramBuildInfo(engine->program, engine->device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
      char* log = (char*)malloc(log_size);
      clGetProgramBuildInfo(engine->program, engine->device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
      printf("OpenCL build error:\n%s\n", log);
      free(log);
  }

  engine->screen_resolution[0] = width;
  engine->screen_resolution[1] = height;

  engine->clearKernel    = clCreateKernel(engine->program, "clear_buffers", NULL);
  engine->vertexKernel   = clCreateKernel(engine->program, "vertex_kernel", NULL);
  engine->fragmentKernel = clCreateKernel(engine->program, "fragment_kernel", NULL);

  engine->frameBuffer = clCreateBuffer(engine->context, CL_MEM_WRITE_ONLY, 
                                      engine->screen_resolution[0] * engine->screen_resolution[1]
                                              * sizeof(Color), NULL, NULL);
  engine->depthBuffer = clCreateBuffer(engine->context, CL_MEM_READ_WRITE,
                                      sizeof(float) * engine->screen_resolution[0]
                                                    * engine->screen_resolution[1],
                                                    NULL, &engine->err);
  engine->trisBuffer = clCreateBuffer(engine->context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                     sizeof(Triangle) * model->numTriangles, model->triangles, &engine->err);
  engine->mvpVertsBuffer  = clCreateBuffer(engine->context, CL_MEM_READ_WRITE,
                                          sizeof(f4) * model->numVertices, NULL, NULL);
  engine->fragPosBuffer  = clCreateBuffer(engine->context, CL_MEM_READ_WRITE, sizeof(f3), NULL, NULL);
  engine->projectionBuffer  = clCreateBuffer(engine->context, CL_MEM_READ_ONLY,
                                            sizeof(f4x4), NULL, &engine->err);
  engine->viewBuffer  = clCreateBuffer(engine->context, CL_MEM_READ_ONLY,
                                      sizeof(f4x4), NULL, &engine->err);
  engine->transformBuffer  = clCreateBuffer(engine->context, CL_MEM_READ_ONLY,
                                           sizeof(f4x4), NULL, &engine->err);
  engine->cameraPosBuffer  = clCreateBuffer(engine->context, CL_MEM_READ_ONLY,
                                           sizeof(f3), NULL, &engine->err);
  engine->textureBuffer = clCreateBuffer(engine->context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                        model->texWidth * model->texHeight * sizeof(Color), model->pixels,
                                        &engine->err);

  clSetKernelArg(engine->clearKernel, 0, sizeof(cl_mem), &engine->frameBuffer);
  clSetKernelArg(engine->clearKernel, 1, sizeof(cl_mem), &engine->depthBuffer);
  clSetKernelArg(engine->clearKernel, 2, sizeof(int), &engine->screen_resolution[0]);
  clSetKernelArg(engine->clearKernel, 3, sizeof(int), &engine->screen_resolution[1]);
  clEnqueueNDRangeKernel(engine->queue, engine->clearKernel, 2, NULL, engine->screen_resolution, NULL, 0, NULL, NULL);

  clSetKernelArg(engine->vertexKernel, 0, sizeof(cl_mem), &engine->trisBuffer);
  clSetKernelArg(engine->vertexKernel, 1, sizeof(cl_mem), &engine->mvpVertsBuffer);
  clSetKernelArg(engine->vertexKernel, 2, sizeof(cl_mem), &engine->projectionBuffer); 
  clSetKernelArg(engine->vertexKernel, 3, sizeof(cl_mem), &engine->viewBuffer); 
  clSetKernelArg(engine->vertexKernel, 4, sizeof(cl_mem), &engine->transformBuffer); 
  clSetKernelArg(engine->vertexKernel, 5, sizeof(int), &model->numTriangles);
  clSetKernelArg(engine->vertexKernel, 6, sizeof(int), &engine->screen_resolution[0]);
  clSetKernelArg(engine->vertexKernel, 7, sizeof(int), &engine->screen_resolution[1]);
  clSetKernelArg(engine->vertexKernel, 8, sizeof(cl_mem), &engine->cameraPosBuffer);
  clSetKernelArg(engine->vertexKernel, 9, sizeof(cl_mem), &engine->fragPosBuffer);

  clSetKernelArg(engine->fragmentKernel, 0, sizeof(cl_mem), &engine->frameBuffer);
  clSetKernelArg(engine->fragmentKernel, 1, sizeof(cl_mem), &engine->trisBuffer);
  clSetKernelArg(engine->fragmentKernel, 2, sizeof(cl_mem), &engine->mvpVertsBuffer);
  clSetKernelArg(engine->fragmentKernel, 3, sizeof(int), &model->numTriangles);
  clSetKernelArg(engine->fragmentKernel, 4, sizeof(int), &engine->screen_resolution[0]);
  clSetKernelArg(engine->fragmentKernel, 5, sizeof(int), &engine->screen_resolution[1]);
  clSetKernelArg(engine->fragmentKernel, 6, sizeof(cl_mem), &engine->depthBuffer);
  clSetKernelArg(engine->fragmentKernel, 7, sizeof(cl_mem), &engine->cameraPosBuffer);
  clSetKernelArg(engine->fragmentKernel, 8, sizeof(cl_mem), &engine->fragPosBuffer);
  clSetKernelArg(engine->fragmentKernel, 9, sizeof(cl_mem), &engine->textureBuffer);
  clSetKernelArg(engine->fragmentKernel, 10, sizeof(int), &model->texWidth);
  clSetKernelArg(engine->fragmentKernel, 11, sizeof(int), &model->texHeight);

  Image img = GenImageColor(engine->screen_resolution[0], engine->screen_resolution[1], engine->clearColor);
  engine->texture = LoadTextureFromImage(img);
  free(img.data); // pixel buffer is managed by OpenCL

  engine->pixelBuffer = (Color*)malloc(engine->screen_resolution[0]
                                       * engine->screen_resolution[1] 
                                       * sizeof(Color));
  
  clEnqueueWriteBuffer(engine->queue, engine->transformBuffer, CL_TRUE, 0,
                      sizeof(f4x4), &model->transform, 0, NULL, NULL);
  clEnqueueWriteBuffer(engine->queue, engine->projectionBuffer, CL_TRUE, 0,
                       sizeof(f4x4), &camera->proj, 0, NULL, NULL);
}

inline void engine_background_color(Engine* engine, Color color)
{
  clSetKernelArg(engine->clearKernel, 4, sizeof(Color), &color);
}

inline void engine_clear_background(Engine* engine)
{
  clEnqueueNDRangeKernel(engine->queue, engine->clearKernel, 2, NULL, engine->screen_resolution, NULL, 0, NULL, NULL);
}

inline void engine_send_camera_matrix(Engine* engine, CustomCamera* camera)
{
  clEnqueueWriteBuffer(engine->queue, engine->cameraPosBuffer, CL_TRUE, 0,
                       sizeof(f3), &camera->Position, 0, NULL, NULL);
  clEnqueueWriteBuffer(engine->queue, engine->viewBuffer, CL_TRUE, 0,
                       sizeof(f4x4), &camera->look_at, 0, NULL, NULL);
}

inline void engine_run_rasterizer(Engine* engine, CustomModel* model)
{
  clEnqueueNDRangeKernel(engine->queue, engine->vertexKernel, 1, NULL,
                         &model->numVertices, NULL, 0, NULL, NULL);
  clEnqueueNDRangeKernel(engine->queue, engine->fragmentKernel, 2, NULL,
                         engine->screen_resolution, NULL, 0, NULL, NULL);
}

inline void engine_read_and_display(Engine* engine)
{
  clEnqueueReadBuffer(engine->queue, engine->frameBuffer, CL_TRUE, 0,
                      engine->screen_resolution[0] * engine->screen_resolution[1] * sizeof(Color),
                      engine->pixelBuffer, 0, NULL, NULL);
  
  clFinish(engine->queue);
  clEnqueueReadBuffer(engine->queue, engine->frameBuffer, CL_TRUE, 0,
                      engine->screen_resolution[0] * engine->screen_resolution[1] * sizeof(Color),
                      engine->pixelBuffer, 0, NULL, NULL);

  UpdateTexture(engine->texture, engine->pixelBuffer);
  BeginDrawing();
  DrawTexture(engine->texture, 0, 0, WHITE);
  EndDrawing();
}

inline void engine_close(Engine* engine)
{
  free(engine->pixelBuffer);

  UnloadTexture(engine->texture);
  CloseWindow();

  clReleaseMemObject(engine->frameBuffer);
  clReleaseMemObject(engine->trisBuffer);
  clReleaseMemObject(engine->mvpVertsBuffer);
  clReleaseKernel(engine->vertexKernel);
  clReleaseKernel(engine->fragmentKernel);
  clReleaseProgram(engine->program);
  clReleaseCommandQueue(engine->queue);
  clReleaseContext(engine->context);

}

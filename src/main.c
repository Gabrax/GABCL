#include <float.h>
#include <limits.h>
#include <stdio.h>
#include "CL/cl.h"
#include "shapes.h"
#include "raylib.h"
#include "camera.h"
#include "CL.h"
#include <stdlib.h>

int main()
{
  InitWindow(800, 600, "GABCL");
  SetTargetFPS(60);
  DisableCursor();
  size_t screen_resolution[2] = {GetScreenWidth(),GetScreenHeight()};

  GAB_Camera camera = {0};
  camera_init(&camera,screen_resolution[0],screen_resolution[1],90.0f,0.01f,1000.0f);

  CustomModel mesh = {0};
  custom_model_load(&mesh, "res/dragon.obj", (Color){0,255,0,255});
  /*model_print_data(&mesh);*/

  CL cl = {0};
  cl_init(&cl, "src/shapes.cl");

  cl_kernel clearKernel   = clCreateKernel(cl.program, "clear_buffers", NULL);
  cl_kernel vertexKernel   = clCreateKernel(cl.program, "vertex_kernel", NULL);
  cl_kernel fragmentKernel = clCreateKernel(cl.program, "fragment_kernel", NULL);

  cl_mem frameBuffer   = clCreateBuffer(cl.context, CL_MEM_WRITE_ONLY, screen_resolution[0] * screen_resolution[1] * sizeof(Color), NULL, NULL);
  cl_mem depthBuffer = clCreateBuffer(cl.context, CL_MEM_READ_WRITE, sizeof(float) * screen_resolution[0] * screen_resolution[1], NULL, &cl.err);
  cl_mem trisBuffer = clCreateBuffer(cl.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(Triangle) * mesh.numTriangles, mesh.triangles, &cl.err);
  cl_mem mvpVertsBuffer  = clCreateBuffer(cl.context, CL_MEM_READ_WRITE, sizeof(f4) * mesh.numVertices, NULL, NULL);
  cl_mem fragPosBuffer  = clCreateBuffer(cl.context, CL_MEM_READ_WRITE, sizeof(f3), NULL, NULL);
  cl_mem projectionBuffer  = clCreateBuffer(cl.context, CL_MEM_READ_ONLY, sizeof(f4x4), NULL, &cl.err);
  cl_mem viewBuffer  = clCreateBuffer(cl.context, CL_MEM_READ_ONLY, sizeof(f4x4), NULL, &cl.err);
  cl_mem transformBuffer  = clCreateBuffer(cl.context, CL_MEM_READ_ONLY, sizeof(f4x4), NULL, &cl.err);
  cl_mem cameraPosBuffer  = clCreateBuffer(cl.context, CL_MEM_READ_ONLY, sizeof(f3), NULL, &cl.err);

  Color clearColor = {0, 0, 0, 255};
  float clearDepth = FLT_MAX;
  clSetKernelArg(clearKernel, 0, sizeof(cl_mem), &frameBuffer);
  clSetKernelArg(clearKernel, 1, sizeof(cl_mem), &depthBuffer);
  clSetKernelArg(clearKernel, 2, sizeof(int), &screen_resolution[0]);
  clSetKernelArg(clearKernel, 3, sizeof(int), &screen_resolution[1]);
  clSetKernelArg(clearKernel, 4, sizeof(Color), &clearColor);
  clSetKernelArg(clearKernel, 5, sizeof(float), &clearDepth);
  clEnqueueNDRangeKernel(cl.queue, clearKernel, 2, NULL, screen_resolution, NULL, 0, NULL, NULL);

  clSetKernelArg(vertexKernel, 0, sizeof(cl_mem), &trisBuffer);
  clSetKernelArg(vertexKernel, 1, sizeof(cl_mem), &mvpVertsBuffer);
  clSetKernelArg(vertexKernel, 2, sizeof(cl_mem), &projectionBuffer); 
  clSetKernelArg(vertexKernel, 3, sizeof(cl_mem), &viewBuffer); 
  clSetKernelArg(vertexKernel, 4, sizeof(cl_mem), &transformBuffer); 
  clSetKernelArg(vertexKernel, 5, sizeof(int), &mesh.numTriangles);
  clSetKernelArg(vertexKernel, 6, sizeof(int), &screen_resolution[0]);
  clSetKernelArg(vertexKernel, 7, sizeof(int), &screen_resolution[1]);
  clSetKernelArg(vertexKernel, 8, sizeof(cl_mem), &cameraPosBuffer);
  clSetKernelArg(vertexKernel, 9, sizeof(cl_mem), &fragPosBuffer);

  clSetKernelArg(fragmentKernel, 0, sizeof(cl_mem), &frameBuffer);
  clSetKernelArg(fragmentKernel, 1, sizeof(cl_mem), &trisBuffer);
  clSetKernelArg(fragmentKernel, 2, sizeof(cl_mem), &mvpVertsBuffer);
  clSetKernelArg(fragmentKernel, 3, sizeof(int), &mesh.numTriangles);
  clSetKernelArg(fragmentKernel, 4, sizeof(int), &screen_resolution[0]);
  clSetKernelArg(fragmentKernel, 5, sizeof(int), &screen_resolution[1]);
  clSetKernelArg(fragmentKernel, 6, sizeof(cl_mem), &depthBuffer);
  clSetKernelArg(fragmentKernel, 7, sizeof(cl_mem), &cameraPosBuffer);
  clSetKernelArg(fragmentKernel, 8, sizeof(cl_mem), &fragPosBuffer);

  Image img = GenImageColor(screen_resolution[0], screen_resolution[1], BLACK);
  Texture2D texture = LoadTextureFromImage(img);
  free(img.data); // pixel buffer is managed by OpenCL

  Color* pixelBuffer = (Color*)malloc(screen_resolution[0] * screen_resolution[1] * sizeof(Color));
        
  f4x4 model = MatTransform((f3){0.0f, 0.0f, 5.0f}, (f3){0.0f, 0.0f, 0.0f}, (f3){0.5f, 0.5f, 0.5f});
  clEnqueueWriteBuffer(cl.queue, transformBuffer, CL_TRUE, 0, sizeof(f4x4), &model, 0, NULL, NULL);
  clEnqueueWriteBuffer(cl.queue, projectionBuffer, CL_TRUE, 0, sizeof(f4x4), &camera.proj, 0, NULL, NULL);

  char str[12];
  while (!WindowShouldClose())
  {
    if (IsKeyDown(KEY_W)) camera_process_keys(&camera, FORWARD);
    if (IsKeyDown(KEY_S)) camera_process_keys(&camera, BACKWARD);
    if (IsKeyDown(KEY_A)) camera_process_keys(&camera, LEFT);
    if (IsKeyDown(KEY_D)) camera_process_keys(&camera, RIGHT);

    camera_update(&camera, GetMouseX(), GetMouseY(), true);

    // 1. Update camera data to GPU
    clEnqueueWriteBuffer(cl.queue, cameraPosBuffer, CL_TRUE, 0, sizeof(f3), &camera.Position, 0, NULL, NULL);
    clEnqueueWriteBuffer(cl.queue, viewBuffer, CL_TRUE, 0, sizeof(f4x4), &camera.look_at, 0, NULL, NULL);

    // 2. Clear buffers (could include color + depth in same kernel)
    clEnqueueNDRangeKernel(cl.queue, clearKernel, 2, NULL, screen_resolution, NULL, 0, NULL, NULL);

    // 4. Draw
    clEnqueueNDRangeKernel(cl.queue, vertexKernel, 1, NULL, &mesh.numVertices, NULL, 0, NULL, NULL);
    clEnqueueNDRangeKernel(cl.queue, fragmentKernel, 2, NULL, screen_resolution, NULL, 0, NULL, NULL);

    // 6. Read back to CPU
    clEnqueueReadBuffer(cl.queue, frameBuffer, CL_TRUE, 0, screen_resolution[0] * screen_resolution[1] * sizeof(Color), pixelBuffer, 0, NULL, NULL);
    
    clFinish(cl.queue);
    clEnqueueReadBuffer(cl.queue, frameBuffer, CL_TRUE, 0, screen_resolution[0] * screen_resolution[1] * sizeof(Color), pixelBuffer, 0, NULL, NULL);

    UpdateTexture(texture, pixelBuffer);
    BeginDrawing();
    DrawTexture(texture, 0, 0, WHITE);
    _itoa(GetFPS(),str,10);
    DrawText(str, 10, 0, 13, RAYWHITE);
    EndDrawing();
  }

  free(pixelBuffer);

  UnloadTexture(texture);
  CloseWindow();

  clReleaseMemObject(frameBuffer);
  clReleaseMemObject(trisBuffer);
  clReleaseMemObject(mvpVertsBuffer);
  clReleaseKernel(vertexKernel);
  clReleaseKernel(fragmentKernel);
  clReleaseProgram(cl.program);
  clReleaseCommandQueue(cl.queue);
  clReleaseContext(cl.context);

  return 0;
}


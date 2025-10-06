#include <stdio.h>
#include <math.h>
#include "algebra.h"
#include "shapes.h"
#include "raylib.h"
#include "camera.h"
#include "CL.h"

#define WIDTH 800
#define HEIGHT 600

int main()
{
  InitWindow(WIDTH, HEIGHT, "GABCL");
  SetTargetFPS(60);

  int width = WIDTH, height = HEIGHT;

  GAB_Camera camera = {0};
  camera.Position = (Vec3){0.0,0.0,0.0};
  camera.Up = (Vec3){0.0,1.0,0.0};
  camera.Front = (Vec3){0.0,0.0,-1.0};
  camera.aspect_ratio = (float)GetScreenWidth()/(float)GetScreenHeight();
  camera.near_plane = 0.01f;
  camera.far_plane = 1000.0f;
  camera.fov = 45.0f;
  camera.fov_rad = 1.0f/tanf(camera.fov * 0.5f/180.0f * 3.14129f);

  Mat4 proj = MatPerspective(camera.fov_rad, camera.aspect_ratio, camera.near_plane, camera.far_plane);

  Mat4 translate = MatTransform((Vec3){0.0,0.0,3.0},(Vec3){0.0,15.0,0.0},(Vec3){0.5,0.5,0.5});

  Mat4 projection = MatMul(proj, translate);
  MatPrint(&projection);

  CustomModel mesh = LoadfromOBJ("res/cube.obj", (Color){0,255,0,255});
  size_t vertexGlobal = mesh.numTriangles * 3;

  CL cl = clInit("src/shapes.cl");

  cl_kernel vertexKernel = clCreateKernel(cl.program, "vertex_kernel", NULL);
  cl_kernel fragmentKernel = clCreateKernel(cl.program, "fragment_kernel", NULL);

  cl_mem clPixels = clCreateBuffer(cl.context, CL_MEM_WRITE_ONLY, WIDTH * HEIGHT * sizeof(Color), NULL, NULL);
  cl_mem clDepth = clCreateBuffer(cl.context, CL_MEM_WRITE_ONLY, WIDTH * HEIGHT * sizeof(float), NULL, NULL);
  cl_mem trisBuffer = clCreateBuffer(cl.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(Triangle) * mesh.numTriangles, mesh.triangles, &cl.err);
  cl_mem projVerts = clCreateBuffer(cl.context, CL_MEM_READ_WRITE, sizeof(Vec3) * mesh.numTriangles * 3, NULL, NULL);
  cl_mem mvpBuffer = clCreateBuffer(cl.context,CL_MEM_READ_ONLY,sizeof(Mat4), NULL, &cl.err);

  clSetKernelArg(vertexKernel, 0, sizeof(cl_mem), &trisBuffer);
  clSetKernelArg(vertexKernel, 1, sizeof(cl_mem), &projVerts);
  clSetKernelArg(vertexKernel, 2, sizeof(int), &mesh.numTriangles);
  clSetKernelArg(vertexKernel, 3, sizeof(int), &width);
  clSetKernelArg(vertexKernel, 4, sizeof(int), &height);
  clSetKernelArg(vertexKernel, 5, sizeof(cl_mem), &mvpBuffer); // only once

  clSetKernelArg(fragmentKernel, 0, sizeof(cl_mem), &clPixels);
  clSetKernelArg(fragmentKernel, 1, sizeof(cl_mem), &clDepth);
  clSetKernelArg(fragmentKernel, 2, sizeof(cl_mem), &trisBuffer);
  clSetKernelArg(fragmentKernel, 3, sizeof(cl_mem), &projVerts);
  clSetKernelArg(fragmentKernel, 4, sizeof(int), &mesh.numTriangles);
  clSetKernelArg(fragmentKernel, 5, sizeof(int), &width);
  clSetKernelArg(fragmentKernel, 6, sizeof(int), &height);

  Image img = GenImageColor(WIDTH, HEIGHT, BLACK);
  Texture2D texture = LoadTextureFromImage(img);
  free(img.data); // pixel buffer is managed by OpenCL

  size_t global[2] = {WIDTH, HEIGHT};
  Color* pixelBuffer = (Color*)malloc(WIDTH * HEIGHT * sizeof(Color));
  float anglex = 0.0f, angley = 0.0f;
  while (!WindowShouldClose())
  {
    clEnqueueWriteBuffer(cl.queue, mvpBuffer, CL_TRUE, 0, sizeof(Mat4), &projection, 0, NULL, NULL);

    clEnqueueNDRangeKernel(cl.queue, vertexKernel, 1, NULL, &vertexGlobal, NULL, 0, NULL, NULL);
    clEnqueueNDRangeKernel(cl.queue, fragmentKernel, 2, NULL, global, NULL, 0, NULL, NULL);

    clEnqueueReadBuffer(cl.queue, clPixels, CL_TRUE, 0, WIDTH * HEIGHT * sizeof(Color), pixelBuffer, 0, NULL, NULL);
    UpdateTexture(texture, pixelBuffer);

    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexture(texture, 0, 0, WHITE);
    EndDrawing();
  }

  free(pixelBuffer);

  UnloadTexture(texture);
  CloseWindow();

  clReleaseMemObject(clPixels);
  clReleaseMemObject(clDepth);
  clReleaseMemObject(trisBuffer);
  clReleaseMemObject(projVerts);
  clReleaseKernel(vertexKernel);
  clReleaseKernel(fragmentKernel);
  clReleaseProgram(cl.program);
  clReleaseCommandQueue(cl.queue);
  clReleaseContext(cl.context);

  return 0;
}


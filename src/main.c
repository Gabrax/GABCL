#include <stdio.h>
#include <math.h>
#include "shapes.h"
#include "raylib.h"
#include "rcamera.h"
#include "CL.h"

#define WIDTH 512
#define HEIGHT 512

int main()
{
  InitWindow(WIDTH, HEIGHT, "GABCL");
  SetTargetFPS(60);

  int width = WIDTH, height = HEIGHT;

  Camera3D camera = {0};  
  camera.position = (Vector3){ 0.0f, 2.0f, 6.0f };  
  camera.target   = (Vector3){ 0.0f, 0.0f, 0.0f };  
  camera.up       = (Vector3){ 0.0f, 1.0f, 0.0f };  
  camera.fovy     = 45.0f;                           
  camera.projection     = CAMERA_PERSPECTIVE;

  CustomModel mesh = make_mesh_from_OBJ("res/cube.obj", (Color){0,255,0,255});
  size_t vertexGlobal = mesh.numTriangles * 3;

  CL cl = clInit("src/shapes.cl");

  cl_kernel vertexKernel = clCreateKernel(cl.program, "vertex_kernel", NULL);
  cl_kernel fragmentKernel = clCreateKernel(cl.program, "fragment_kernel", NULL);

  cl_mem clPixels = clCreateBuffer(cl.context, CL_MEM_WRITE_ONLY, WIDTH * HEIGHT * sizeof(Color), NULL, NULL);
  cl_mem clDepth = clCreateBuffer(cl.context, CL_MEM_WRITE_ONLY, WIDTH * HEIGHT * sizeof(float), NULL, NULL);
  cl_mem trisBuffer = clCreateBuffer(cl.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(Triangle) * mesh.numTriangles, mesh.triangles, &cl.err);
  cl_mem projVerts = clCreateBuffer(cl.context, CL_MEM_READ_WRITE, sizeof(Vector3) * mesh.numTriangles * 3, NULL, NULL);

  clSetKernelArg(vertexKernel, 0, sizeof(cl_mem), &trisBuffer);
  clSetKernelArg(vertexKernel, 1, sizeof(cl_mem), &projVerts);
  clSetKernelArg(vertexKernel, 2, sizeof(int), &mesh.numTriangles);
  clSetKernelArg(vertexKernel, 4, sizeof(int), &width);
  clSetKernelArg(vertexKernel, 5, sizeof(int), &height);

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

  float t = 0.0f;
  Matrix view = GetCameraViewMatrix(&camera);
  Matrix proj = GetCameraProjectionMatrix(&camera, (float)GetScreenWidth()/GetScreenHeight());
  Matrix viewProj = MatrixMultiply(proj, view); 

  size_t global[2] = {WIDTH, HEIGHT};
  Color* pixelBuffer = (Color*)malloc(WIDTH * HEIGHT * sizeof(Color));

  while (!WindowShouldClose())
  {
    UpdateCamera(&camera, CAMERA_FIRST_PERSON);

    clSetKernelArg(vertexKernel, 3, sizeof(float), &t);
    clEnqueueNDRangeKernel(cl.queue, vertexKernel, 1, NULL, &vertexGlobal, NULL, 0, NULL, NULL);
    clEnqueueNDRangeKernel(cl.queue, fragmentKernel, 2, NULL, global, NULL, 0, NULL, NULL);

    clEnqueueReadBuffer(cl.queue, clPixels, CL_TRUE, 0, WIDTH * HEIGHT * sizeof(Color), pixelBuffer, 0, NULL, NULL);
    UpdateTexture(texture, pixelBuffer);

    t += 0.02f;

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


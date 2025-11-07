#include <float.h>
#include <limits.h>
#include <stdio.h>
#include "CL/cl.h"
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
  DisableCursor();

  int width = WIDTH, height = HEIGHT;

  GAB_Camera camera = {0};
  camera.Position = (f3){0.0f, 0.0f, 0.0f};
  camera.WorldUp = (f3){0.0f, 1.0f, 0.0f};
  camera.Front = (f3){0.0f, 0.0f, 1.0f};
  camera.aspect_ratio = (float)GetScreenWidth() / (float)GetScreenHeight();
  camera.near_plane = 0.01f;
  camera.far_plane = 1000.0f;
  camera.fov = 60.0f;
  camera.fov_rad = DegToRad(camera.fov);  
  camera.proj = MatPerspective(camera.fov_rad, camera.aspect_ratio, camera.near_plane, camera.far_plane);
  camera.look_at = MatLookAt(camera.Position, f3Add(camera.Position, camera.Front), camera.WorldUp);
  camera.yaw = 90.0f;
  camera.pitch = 0.0f;
  camera.speed = 2.0f;
  camera.sens = 0.1f;
  camera.lastX = WIDTH / 2.0f;
  camera.lastY = HEIGHT / 2.0f;
  camera.firstMouse = true;

  CustomModel mesh = LoadModel_Assimp("res/dragon.obj", (Color){0,255,0,255});
  /*PrintMesh(&mesh);*/
  size_t vertexGlobal = mesh.numTriangles * 3;

  CL cl = clInit("src/shapes.cl");

  cl_kernel clearKernel   = clCreateKernel(cl.program, "clear_pixels", NULL);
  cl_kernel vertexKernel   = clCreateKernel(cl.program, "vertex_kernel", NULL);
  cl_kernel fragmentKernel = clCreateKernel(cl.program, "fragment_kernel", NULL);

  cl_mem frameBuffer   = clCreateBuffer(cl.context, CL_MEM_WRITE_ONLY, WIDTH * HEIGHT * sizeof(Color), NULL, NULL);
  cl_mem depthBuffer = clCreateBuffer(cl.context, CL_MEM_READ_WRITE, sizeof(float) * WIDTH * HEIGHT, NULL, &cl.err);
  cl_mem trisBuffer = clCreateBuffer(cl.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(Triangle) * mesh.numTriangles, mesh.triangles, &cl.err);
  cl_mem mvpVertsBuffer  = clCreateBuffer(cl.context, CL_MEM_READ_WRITE, sizeof(f4) * mesh.numTriangles * 3, NULL, NULL);
  cl_mem fragPosBuffer  = clCreateBuffer(cl.context, CL_MEM_READ_WRITE, sizeof(f3), NULL, NULL);
  cl_mem projectionBuffer  = clCreateBuffer(cl.context, CL_MEM_READ_ONLY, sizeof(f4x4), NULL, &cl.err);
  cl_mem viewBuffer  = clCreateBuffer(cl.context, CL_MEM_READ_ONLY, sizeof(f4x4), NULL, &cl.err);
  cl_mem transformBuffer  = clCreateBuffer(cl.context, CL_MEM_READ_ONLY, sizeof(f4x4), NULL, &cl.err);
  cl_mem cameraPosBuffer  = clCreateBuffer(cl.context, CL_MEM_READ_ONLY, sizeof(f3), NULL, &cl.err);

  clSetKernelArg(clearKernel, 0, sizeof(cl_mem), &frameBuffer);
  clSetKernelArg(clearKernel, 1, sizeof(int), &width);
  clSetKernelArg(clearKernel, 2, sizeof(int), &height);

  clSetKernelArg(vertexKernel, 0, sizeof(cl_mem), &trisBuffer);
  clSetKernelArg(vertexKernel, 1, sizeof(cl_mem), &mvpVertsBuffer);
  clSetKernelArg(vertexKernel, 2, sizeof(cl_mem), &projectionBuffer); 
  clSetKernelArg(vertexKernel, 3, sizeof(cl_mem), &viewBuffer); 
  clSetKernelArg(vertexKernel, 4, sizeof(cl_mem), &transformBuffer); 
  clSetKernelArg(vertexKernel, 5, sizeof(int), &mesh.numTriangles);
  clSetKernelArg(vertexKernel, 6, sizeof(int), &width);
  clSetKernelArg(vertexKernel, 7, sizeof(int), &height);
  clSetKernelArg(vertexKernel, 8, sizeof(cl_mem), &cameraPosBuffer);
  clSetKernelArg(vertexKernel, 9, sizeof(cl_mem), &fragPosBuffer);

  clSetKernelArg(fragmentKernel, 0, sizeof(cl_mem), &frameBuffer);
  clSetKernelArg(fragmentKernel, 1, sizeof(cl_mem), &trisBuffer);
  clSetKernelArg(fragmentKernel, 2, sizeof(cl_mem), &mvpVertsBuffer);
  clSetKernelArg(fragmentKernel, 3, sizeof(int), &mesh.numTriangles);
  clSetKernelArg(fragmentKernel, 4, sizeof(int), &width);
  clSetKernelArg(fragmentKernel, 5, sizeof(int), &height);
  clSetKernelArg(fragmentKernel, 6, sizeof(cl_mem), &depthBuffer);
  clSetKernelArg(fragmentKernel, 7, sizeof(cl_mem), &cameraPosBuffer);
  clSetKernelArg(fragmentKernel, 8, sizeof(cl_mem), &fragPosBuffer);

  Image img = GenImageColor(WIDTH, HEIGHT, BLACK);
  Texture2D texture = LoadTextureFromImage(img);
  free(img.data); // pixel buffer is managed by OpenCL

  size_t global[2] = {WIDTH, HEIGHT};
  Color* pixelBuffer = (Color*)malloc(WIDTH * HEIGHT * sizeof(Color));
        
  f4x4 model = MatTransform((f3){0.0f, 0.0f, 5.0f}, (f3){0.0f, 0.0f, 0.0f}, (f3){0.5f, 0.5f, 0.5f});
  clEnqueueWriteBuffer(cl.queue, transformBuffer, CL_TRUE, 0, sizeof(f4x4), &model, 0, NULL, NULL);
  clEnqueueWriteBuffer(cl.queue, projectionBuffer, CL_TRUE, 0, sizeof(f4x4), &camera.proj, 0, NULL, NULL);

  float* hostDepth = (float*)malloc(sizeof(float) * WIDTH * HEIGHT);

  float t = 0;
  while (!WindowShouldClose())
  {
    float deltaTime = GetFrameTime();
    if (IsKeyDown(KEY_W)) ProcessKeyboard(&camera, FORWARD, deltaTime);
    if (IsKeyDown(KEY_S)) ProcessKeyboard(&camera, BACKWARD, deltaTime);
    if (IsKeyDown(KEY_A)) ProcessKeyboard(&camera, LEFT, deltaTime);
    if (IsKeyDown(KEY_D)) ProcessKeyboard(&camera, RIGHT, deltaTime);

    updateCamera(&camera, GetMouseX(), GetMouseY(), true);

    /*t+= 1;*/
    /*model = MatTransform((Vec3){0.0f, 0.0f, 5.0f}, (Vec3){0.0f, t, 0.0f}, (Vec3){2.5f, 2.5f, 2.5f});*/
    /*clEnqueueWriteBuffer(cl.queue, transformBuffer, CL_TRUE, 0, sizeof(Mat4), &model, 0, NULL, NULL);*/

    for(int i = 0; i < WIDTH*HEIGHT; i++) hostDepth[i] = FLT_MAX;
    clEnqueueWriteBuffer(cl.queue, depthBuffer, CL_TRUE, 0, sizeof(float) * WIDTH * HEIGHT, hostDepth, 0, NULL, NULL);
    clEnqueueWriteBuffer(cl.queue, cameraPosBuffer, CL_TRUE, 0, sizeof(f3), &camera.Position, 0, NULL, NULL);
    clEnqueueWriteBuffer(cl.queue, viewBuffer, CL_TRUE, 0, sizeof(f4x4), &camera.look_at, 0, NULL, NULL);

    clEnqueueNDRangeKernel(cl.queue, clearKernel, 2, NULL, global, NULL, 0, NULL, NULL);
    clEnqueueNDRangeKernel(cl.queue, vertexKernel, 1, NULL, &vertexGlobal, NULL, 0, NULL, NULL);
    clEnqueueNDRangeKernel(cl.queue, fragmentKernel, 2, NULL, global, NULL, 0, NULL, NULL);

    clEnqueueReadBuffer(cl.queue, frameBuffer, CL_TRUE, 0, WIDTH * HEIGHT * sizeof(Color), pixelBuffer, 0, NULL, NULL);

    UpdateTexture(texture, pixelBuffer);
    BeginDrawing();
    DrawTexture(texture, 0, 0, WHITE);
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


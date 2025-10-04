#include <stdio.h>
#include <math.h>
#include "shapes.h"
#include "raylib.h"
#include "rcamera.h"
#include "CL.h"

#define WIDTH 512
#define HEIGHT 512

void PrintMatrix(const Matrix* mat)
{
    printf("[ %f, %f, %f, %f ]\n", mat->m0, mat->m4, mat->m8,  mat->m12);
    printf("[ %f, %f, %f, %f ]\n", mat->m1, mat->m5, mat->m9,  mat->m13);
    printf("[ %f, %f, %f, %f ]\n", mat->m2, mat->m6, mat->m10, mat->m14);
    printf("[ %f, %f, %f, %f ]\n", mat->m3, mat->m7, mat->m11, mat->m15);
}

int main()
{
  InitWindow(WIDTH, HEIGHT, "GABCL");
  SetTargetFPS(60);

  int width = WIDTH, height = HEIGHT;

  Camera3D camera = {0};  
  camera.position = (Vector3){ 0.0f, 0.0f, -10.0f };  
  camera.target   = (Vector3){ 0.0f, 0.0f, 1.0f };  
  camera.up       = (Vector3){ 0.0f, 1.0f, 0.0f };  
  camera.fovy     = 45.0f;                           
  camera.projection     = CAMERA_PERSPECTIVE;

  CustomModel mesh = make_mesh_from_OBJ("res/cube.obj", (Color){0,255,0,255});
  size_t vertexGlobal = mesh.numTriangles * 3;

  Matrix modelTransform = MatrixIdentity();
  Matrix scaleMat = MatrixScale(0.5f,0.5f,0.5f);
  Matrix rotX = MatrixRotateX(0.0f * DEG2RAD);
  Matrix rotY = MatrixRotateY(0.0f * DEG2RAD);
  Matrix rotZ = MatrixRotateZ(0.0f * DEG2RAD);
  Matrix rotMat = MatrixMultiply(rotY, MatrixMultiply(rotX, rotZ));
  Matrix transMat = MatrixTranslate(0.0f, 0.0f, 0.0f);
  modelTransform = MatrixMultiply(transMat, MatrixMultiply(rotMat, scaleMat));

  CL cl = clInit("src/shapes.cl");

  cl_kernel vertexKernel = clCreateKernel(cl.program, "vertex_kernel", NULL);
  cl_kernel fragmentKernel = clCreateKernel(cl.program, "fragment_kernel", NULL);

  cl_mem clPixels = clCreateBuffer(cl.context, CL_MEM_WRITE_ONLY, WIDTH * HEIGHT * sizeof(Color), NULL, NULL);
  cl_mem clDepth = clCreateBuffer(cl.context, CL_MEM_WRITE_ONLY, WIDTH * HEIGHT * sizeof(float), NULL, NULL);
  cl_mem trisBuffer = clCreateBuffer(cl.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(Triangle) * mesh.numTriangles, mesh.triangles, &cl.err);
  cl_mem projVerts = clCreateBuffer(cl.context, CL_MEM_READ_WRITE, sizeof(Vector3) * mesh.numTriangles * 3, NULL, NULL);
  cl_mem mvpBuffer = clCreateBuffer(cl.context,CL_MEM_READ_ONLY,sizeof(Matrix), NULL, &cl.err);

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

  Matrix proj = GetCameraProjectionMatrix(&camera, (float)GetScreenWidth()/GetScreenHeight());

  size_t global[2] = {WIDTH, HEIGHT};
  Color* pixelBuffer = (Color*)malloc(WIDTH * HEIGHT * sizeof(Color));

  while (!WindowShouldClose())
  {
    UpdateCamera(&camera, CAMERA_FIRST_PERSON);
    Matrix view = GetCameraViewMatrix(&camera);
    Matrix proj = GetCameraProjectionMatrix(&camera, (float)WIDTH / HEIGHT);
    Matrix mv   = MatrixMultiply(view, modelTransform);
    Matrix mvp  = MatrixMultiply(proj, mv);   // projection * view * model
    /*printf("MODEL\n"); PrintMatrix(&modelTransform);    */
    /*printf("VIEW\n"); PrintMatrix(&view);    */
    /*printf("PROJ\n"); PrintMatrix(&proj);    */
    /*printf("MVP\n"); PrintMatrix(&mvp);    */

    clEnqueueWriteBuffer(cl.queue, mvpBuffer, CL_TRUE, 0, sizeof(Matrix), &mvp, 0, NULL, NULL);

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


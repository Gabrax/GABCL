#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <CL/cl.h>
#include "window.h"  
#include "algebra.h"   
#include "shapes.h"
#include "utils.h"
#include "camera.h"

#define WIDTH 800
#define HEIGHT 600

int main()
{
    Camera camera = {0};
    camera_init(&camera, (Vec3F){0.0f,0.0f,0.0f}, WIDTH, HEIGHT, 90.0f, 0.001f, 1000.0f);

    Mesh mesh = {0}; 

    arrpush(mesh.triangles, make_triangle((Vec3F){0,0,0}, (Vec3F){1,0,0}, (Vec3F){1,1,0}));
    arrpush(mesh.triangles, make_triangle((Vec3F){0,0,0}, (Vec3F){1,1,0}, (Vec3F){0,1,0}));
    // EAST (+X)
    arrpush(mesh.triangles, make_triangle((Vec3F){1,0,0}, (Vec3F){1,0,1}, (Vec3F){1,1,1}));
    arrpush(mesh.triangles, make_triangle((Vec3F){1,0,0}, (Vec3F){1,1,1}, (Vec3F){1,1,0}));
    // NORTH (+Z)
    arrpush(mesh.triangles, make_triangle((Vec3F){1,0,1}, (Vec3F){0,0,1}, (Vec3F){0,1,1}));
    arrpush(mesh.triangles, make_triangle((Vec3F){1,0,1}, (Vec3F){0,1,1}, (Vec3F){1,1,1}));
    // WEST (-X)
    arrpush(mesh.triangles, make_triangle((Vec3F){0,0,1}, (Vec3F){0,0,0}, (Vec3F){0,1,0}));
    arrpush(mesh.triangles, make_triangle((Vec3F){0,0,1}, (Vec3F){0,1,0}, (Vec3F){0,1,1}));
    // TOP (+Y)
    arrpush(mesh.triangles, make_triangle((Vec3F){0,1,0}, (Vec3F){1,1,0}, (Vec3F){1,1,1}));
    arrpush(mesh.triangles, make_triangle((Vec3F){0,1,0}, (Vec3F){1,1,1}, (Vec3F){0,1,1}));
    // BOTTOM (-Y)
    arrpush(mesh.triangles, make_triangle((Vec3F){0,0,1}, (Vec3F){1,0,1}, (Vec3F){1,0,0}));
    arrpush(mesh.triangles, make_triangle((Vec3F){0,0,1}, (Vec3F){1,0,0}, (Vec3F){0,0,0}));

    Window window;
    window_init(&window, "GABCL", WIDTH, HEIGHT);

    Shape shapes[NUM_SHAPES];

    Mat4F mat = mat4_identity();
    mat4_scale(&mat, (Vec3F){100,100,100});
    mat4_rotate(&mat, 0.0f, (Vec3F){0,0,1});
    mat4_translate(&mat, (Vec3F){100,100,0});
    shapes[0].type  = 0; // square
    shapes[0].transform = mat;
    shapes[0].color = 0xFFFF0000u;

    Mat4F mat1 = mat4_identity();
    mat4_scale(&mat1, (Vec3F){100,100,100});
    mat4_rotate(&mat1, 0.0f, (Vec3F){0,0,1});
    mat4_translate(&mat1, (Vec3F){200,100,0});
    shapes[1].type  = 1; // circle
    shapes[1].transform = mat1;
    shapes[1].color = 0xFF00FF00u;

    Mat4F mat2 = mat4_identity();
    mat4_scale(&mat2, (Vec3F){100,100,100});
    mat4_rotate(&mat2, 0.0f, (Vec3F){0,0,1});
    mat4_translate(&mat2, (Vec3F){300,100,0});
    shapes[2].type  = 2; // triangle
    shapes[2].transform = mat2;
    shapes[2].color = 0xFF0000FFu;

    cl_int err;
    cl_platform_id platform; cl_device_id device;
    clGetPlatformIDs(1,&platform,NULL);
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    cl_context ctx = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    cl_command_queue q = clCreateCommandQueue(ctx, device, 0, &err);

    char* src = loadKernel("src/shapes.cl");
    cl_program prog = clCreateProgramWithSource(ctx,1,(const char**)&src,NULL,&err);
    err = clBuildProgram(prog,1,&device,NULL,NULL,NULL);
    if(err != CL_SUCCESS){
        size_t logSize;
        clGetProgramBuildInfo(prog, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
        char* log = malloc(logSize+1);
        clGetProgramBuildInfo(prog, device, CL_PROGRAM_BUILD_LOG, logSize, log, NULL);
        log[logSize] = 0;
        printf("Build log:\n%s\n", log);
        free(log);
        return 1;
    }

    cl_kernel kdraw = clCreateKernel(prog, "draw_shapes", &err);

    unsigned int* pixels = malloc(WIDTH*HEIGHT*sizeof(unsigned int));
    cl_mem bufPixels = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY, WIDTH*HEIGHT*sizeof(unsigned int), NULL, &err);
    cl_mem bufShapes = clCreateBuffer(ctx, CL_MEM_READ_ONLY, sizeof(Shape)*NUM_SHAPES, NULL, &err);

    clSetKernelArg(kdraw,0,sizeof(cl_mem),&bufPixels);
    clSetKernelArg(kdraw,1,sizeof(cl_mem),&bufShapes);
    int num_shapes = NUM_SHAPES;
    clSetKernelArg(kdraw,2,sizeof(int),&num_shapes);
    int render_width  = WIDTH;
    int render_height = HEIGHT;
    clSetKernelArg(kdraw,3,sizeof(int),&render_width);
    clSetKernelArg(kdraw,4,sizeof(int),&render_height);

    size_t global[2] = { (size_t)WIDTH, (size_t)HEIGHT };

    MSG msg;
    while (1)
    {
      while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
      {
        if(msg.message == WM_QUIT) goto cleanup;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }

      camera_update(&window, &camera, 5.0f, 1.0f);

      if(GetAsyncKeyState(VK_LEFT)  & 0x8000) mat4_translate(&shapes[0].transform,(Vec3F){-20,0,0});
      if(GetAsyncKeyState(VK_RIGHT) & 0x8000) mat4_translate(&shapes[0].transform,(Vec3F){20,0,0});
      if(GetAsyncKeyState(VK_UP)    & 0x8000) mat4_translate(&shapes[0].transform,(Vec3F){0,20,0});
      if(GetAsyncKeyState(VK_DOWN)  & 0x8000) mat4_translate(&shapes[0].transform,(Vec3F){0,-20,0});
      if(GetAsyncKeyState('Q')      & 0x8000) mat4_rotate(&shapes[0].transform, -45.0f,(Vec3F){0,0,1});
      if(GetAsyncKeyState('E')      & 0x8000) mat4_rotate(&shapes[0].transform, 45.0f,(Vec3F){0,0,1});

      clEnqueueWriteBuffer(q, bufShapes, CL_TRUE, 0, sizeof(Shape)*NUM_SHAPES, shapes, 0, NULL, NULL);

      clEnqueueNDRangeKernel(q, kdraw, 2, NULL, global, NULL, 0, NULL, NULL);
      clFinish(q);

      clEnqueueReadBuffer(q, bufPixels, CL_TRUE, 0, sizeof(unsigned int)*WIDTH*HEIGHT, pixels, 0, NULL, NULL);

      window_render(&window, pixels);

      Sleep(16);
    }

cleanup:
    free(pixels);
    clReleaseMemObject(bufPixels);
    clReleaseMemObject(bufShapes);
    clReleaseKernel(kdraw);
    clReleaseProgram(prog);
    clReleaseCommandQueue(q);
    clReleaseContext(ctx);
    return 0;
}

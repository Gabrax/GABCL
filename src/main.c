#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <CL/cl.h>
#include "window.h"  // Your window helper
#include "math.h"   // Your Mat4F helpers

#define WIDTH 800
#define HEIGHT 600
#define NUM_SHAPES 3
#define SHAPE_SQUARE 0
#define SHAPE_CIRCLE 1
#define SHAPE_TRIANGLE 2

typedef struct {
    int type;
    Mat4F model;       
    unsigned int color;
} Shape;

typedef struct {
    int type;
    float row1[4];
    float row2[4];
    float row3[4];
    float row4[4];
    unsigned int color;
} ShapeGPU;

static inline void copy_matrix_to_shape(Mat4F* m, ShapeGPU* sh) {
    sh->row1[0] = m->x1; sh->row1[1] = m->x2; sh->row1[2] = m->x3; sh->row1[3] = m->x4;
    sh->row2[0] = m->y1; sh->row2[1] = m->y2; sh->row2[2] = m->y3; sh->row2[3] = m->y4;
    sh->row3[0] = m->z1; sh->row3[1] = m->z2; sh->row3[2] = m->z3; sh->row3[3] = m->z4;
    sh->row4[0] = m->w1; sh->row4[1] = m->w2; sh->row4[2] = m->w3; sh->row4[3] = m->w4;
}

static inline void shape_translate(Shape* s, float dx, float dy) {
    s->model = mat4_mul(mat4_translate(dx, dy, 0), s->model);
}
static inline void shape_rotate(Shape* s, float rad) {
    // Assuming row-major 4x4 matrix with translation in last column
    float cx = s->model.x4;
    float cy = s->model.y4;

    Mat4F T1 = mat4_translate(-cx, -cy, 0);
    Mat4F R  = mat4_rotate_z(rad);
    Mat4F T2 = mat4_translate(cx, cy, 0);

    s->model = mat4_mul(T2, mat4_mul(R, mat4_mul(T1, s->model)));
}
static inline void shape_scale(Shape* s, float sFactor) {
    s->model = mat4_mul(mat4_scale(sFactor, sFactor, 1), s->model);
}

char* loadKernel(const char* filename) {
    FILE* f = fopen(filename,"rb");
    if(!f){ perror("open kernel"); exit(1);}
    fseek(f,0,SEEK_END);
    size_t size = ftell(f);
    fseek(f,0,SEEK_SET);
    char* src=(char*)malloc(size+1);
    fread(src,1,size,f);
    src[size]='\0';
    fclose(f);
    return src;
}

int main()
{
    Window window;
    window_init(&window, "GABCL", WIDTH, HEIGHT);

    Shape shapes[NUM_SHAPES];
    shapes[0].type  = SHAPE_SQUARE;
    shapes[0].model = mat4_mul(mat4_translate(100,100,0), mat4_scale(50,50,1));
    shapes[0].color = 0xFFFF0000u;

    shapes[1].type  = SHAPE_CIRCLE;
    shapes[1].model = mat4_mul(mat4_translate(300,200,0), mat4_scale(50,50,1));
    shapes[1].color = 0xFF00FF00u;

    shapes[2].type  = SHAPE_TRIANGLE;
    shapes[2].model = mat4_mul(mat4_translate(500,300,0), mat4_scale(60,60,1));
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

    int num_shapes = NUM_SHAPES, render_width = WIDTH, render_height = HEIGHT;

    cl_kernel kdraw = clCreateKernel(prog, "draw_shapes", &err);

    unsigned int* pixels = malloc(WIDTH*HEIGHT*sizeof(unsigned int));
    cl_mem bufPixels = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY, WIDTH*HEIGHT*sizeof(unsigned int), NULL, &err);
    cl_mem bufShapes = clCreateBuffer(ctx, CL_MEM_READ_ONLY, sizeof(ShapeGPU)*NUM_SHAPES, NULL, &err);

    clSetKernelArg(kdraw,0,sizeof(cl_mem),&bufPixels);
    clSetKernelArg(kdraw,1,sizeof(cl_mem),&bufShapes);
    clSetKernelArg(kdraw,2,sizeof(int),&num_shapes);
    clSetKernelArg(kdraw,3,sizeof(int),&render_width);
    clSetKernelArg(kdraw,4,sizeof(int),&render_height);
    ShapeGPU shapes_gpu[NUM_SHAPES];
    size_t global[2] = { (size_t)WIDTH, (size_t)HEIGHT };

    MSG msg;
    while (1) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if(msg.message == WM_QUIT) goto cleanup;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if(GetAsyncKeyState(VK_LEFT) & 0x8000)  shape_translate(&shapes[0], -2, 0);
        if(GetAsyncKeyState(VK_RIGHT)& 0x8000)  shape_translate(&shapes[0],  2, 0);
        if(GetAsyncKeyState(VK_UP)   & 0x8000)  shape_translate(&shapes[0], 0, -2);
        if(GetAsyncKeyState(VK_DOWN) & 0x8000)  shape_translate(&shapes[0], 0,  2);
        if(GetAsyncKeyState('Q')     & 0x8000)  shape_rotate(&shapes[0], -0.005f);
        if(GetAsyncKeyState('E')     & 0x8000)  shape_rotate(&shapes[0],  0.005f);

        for(int i=0;i<NUM_SHAPES;i++){
            Mat4F inv = mat4_inverse_affine(shapes[i].model);
            copy_matrix_to_shape(&inv, &shapes_gpu[i]);
            shapes_gpu[i].type  = shapes[i].type;
            shapes_gpu[i].color = shapes[i].color;
        }

        clEnqueueWriteBuffer(q, bufShapes, CL_TRUE, 0, sizeof(ShapeGPU)*NUM_SHAPES, shapes_gpu, 0, NULL, NULL);
        clEnqueueNDRangeKernel(q, kdraw, 2, NULL, global, NULL, 0, NULL, NULL);
        clFinish(q);
        clEnqueueReadBuffer(q, bufPixels, CL_TRUE, 0, sizeof(unsigned int)*WIDTH*HEIGHT, pixels, 0, NULL, NULL);

        window_render(&window, pixels);
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

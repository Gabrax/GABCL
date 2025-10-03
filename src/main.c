#include <windows.h>
#include <CL/cl.h>
#include <stdio.h>
#include <math.h>
#include "algebra.h"
#include "utils.h"
#include "window.h"
#include "shapes.h"

#define WIDTH 512
#define HEIGHT 512

Vec3F cubeVerts[36] = {
    // front face (z = -0.5)
    {-0.5f,-0.5f,-0.5f}, {0.5f,-0.5f,-0.5f}, {0.5f,0.5f,-0.5f},
    {-0.5f,-0.5f,-0.5f}, {0.5f,0.5f,-0.5f}, {-0.5f,0.5f,-0.5f},

    // back face (z = 0.5)
    {-0.5f,-0.5f,0.5f}, {0.5f,0.5f,0.5f}, {0.5f,-0.5f,0.5f},
    {-0.5f,-0.5f,0.5f}, {-0.5f,0.5f,0.5f}, {0.5f,0.5f,0.5f},

    // bottom face (y = -0.5)
    {-0.5f,-0.5f,-0.5f}, {0.5f,-0.5f,-0.5f}, {0.5f,-0.5f,0.5f},
    {-0.5f,-0.5f,-0.5f}, {0.5f,-0.5f,0.5f}, {-0.5f,-0.5f,0.5f},

    // top face (y = 0.5)
    {-0.5f,0.5f,-0.5f}, {0.5f,0.5f,0.5f}, {0.5f,0.5f,-0.5f},
    {-0.5f,0.5f,-0.5f}, {-0.5f,0.5f,0.5f}, {0.5f,0.5f,0.5f},

    // left face (x = -0.5)
    {-0.5f,-0.5f,-0.5f}, {-0.5f,0.5f,-0.5f}, {-0.5f,0.5f,0.5f},
    {-0.5f,-0.5f,-0.5f}, {-0.5f,0.5f,0.5f}, {-0.5f,-0.5f,0.5f},

    // right face (x = 0.5)
    {0.5f,-0.5f,-0.5f}, {0.5f,0.5f,0.5f}, {0.5f,0.5f,-0.5f},
    {0.5f,-0.5f,-0.5f}, {0.5f,-0.5f,0.5f}, {0.5f,0.5f,0.5f}
};

int main()
{
    Mesh cube = make_mesh_from_vertices(cubeVerts, 36, 0xFF00FFFFu); 
    Window window = createWindow(WIDTH, HEIGHT); 

    int width = WIDTH, height = HEIGHT;
    size_t global[2] = {WIDTH, HEIGHT};
    size_t vertexGlobal = cube.numTriangles * 3;

    cl_platform_id platform;
    cl_device_id device;
    clGetPlatformIDs(1, &platform, NULL);
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);

    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
    cl_command_queue queue = clCreateCommandQueue(context, device, 0, NULL);
    cl_int err;

    const char* kernelSource = loadKernel("src/shapes.cl"); 
    cl_program program = clCreateProgramWithSource(context, 1, &kernelSource, NULL, &err);
    if (err != CL_SUCCESS) { printf("Error creating program: %d\n", err); }

    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        size_t log_size;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        char* log = (char*)malloc(log_size);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
        printf("OpenCL build error:\n%s\n", log);
        free(log);
        return 1;
    }

    cl_kernel vertexKernel = clCreateKernel(program, "vertex_kernel", NULL);
    cl_kernel fragmentKernel = clCreateKernel(program, "fragment_kernel", NULL);

    cl_mem clPixels = clCreateBuffer(context, CL_MEM_WRITE_ONLY, WIDTH * HEIGHT * sizeof(Pixel), NULL, NULL);
    cl_mem clDepth = clCreateBuffer(context, CL_MEM_WRITE_ONLY, WIDTH * HEIGHT * sizeof(float), NULL, NULL);
    cl_mem trisBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(Triangle) * cube.numTriangles, cube.triangles, &err);
    cl_mem projVerts = clCreateBuffer(context, CL_MEM_READ_WRITE,sizeof(Vec3F) * cube.numTriangles * 3, NULL, NULL);

    clSetKernelArg(vertexKernel, 0, sizeof(cl_mem), &trisBuffer);
    clSetKernelArg(vertexKernel, 1, sizeof(cl_mem), &projVerts);
    clSetKernelArg(vertexKernel, 2, sizeof(int), &cube.numTriangles);
    clSetKernelArg(vertexKernel, 4, sizeof(int), &width);
    clSetKernelArg(vertexKernel, 5, sizeof(int), &height);

    clSetKernelArg(fragmentKernel, 0, sizeof(cl_mem), &clPixels);
    clSetKernelArg(fragmentKernel, 1, sizeof(cl_mem), &clDepth);
    clSetKernelArg(fragmentKernel, 2, sizeof(cl_mem), &trisBuffer);
    clSetKernelArg(fragmentKernel, 3, sizeof(cl_mem), &projVerts);
    clSetKernelArg(fragmentKernel, 4, sizeof(int), &cube.numTriangles);
    clSetKernelArg(fragmentKernel, 5, sizeof(int), &width);
    clSetKernelArg(fragmentKernel, 6, sizeof(int), &height);

    float t = 0.0f;
    MSG msg = {0};
    while (msg.message != WM_QUIT)
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        clSetKernelArg(vertexKernel, 3, sizeof(float), &t);

        clEnqueueNDRangeKernel(queue, vertexKernel, 1, NULL, &vertexGlobal, NULL, 0, NULL, NULL);
        clEnqueueNDRangeKernel(queue, fragmentKernel, 2, NULL, global, NULL, 0, NULL, NULL);

        clEnqueueReadBuffer(queue, clPixels, CL_TRUE, 0, WIDTH * HEIGHT * sizeof(Pixel), window.pixelBuffer, 0, NULL, NULL);

        InvalidateRect(window.hwnd, NULL, FALSE);
        t += 0.02f;

        Sleep(16);
    }

    clReleaseMemObject(clPixels);
    clReleaseMemObject(clDepth);
    clReleaseMemObject(trisBuffer);
    clReleaseMemObject(projVerts);
    clReleaseKernel(vertexKernel);
    clReleaseKernel(fragmentKernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    destroyWindow(&window);
    return 0;
}


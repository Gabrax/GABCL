#include <windows.h>
#include <CL/cl.h>
#include <stdio.h>
#include <math.h>

#define WIDTH 512
#define HEIGHT 512

typedef struct { float x, y, z; } Vec3;
typedef struct { unsigned char r, g, b, a; } uchar4;

const float cubeVerts[8][3] = {
    {-0.5f,-0.5f,-0.5f},{0.5f,-0.5f,-0.5f},{0.5f,0.5f,-0.5f},{-0.5f,0.5f,-0.5f},
    {-0.5f,-0.5f,0.5f},{0.5f,-0.5f,0.5f},{0.5f,0.5f,0.5f},{-0.5f,0.5f,0.5f}
};

const int cubeTris[12][3] = {
    {0,1,2},{0,2,3},{4,5,6},{4,6,7},
    {0,1,5},{0,5,4},{2,3,7},{2,7,6},
    {0,3,7},{0,7,4},{1,2,6},{1,6,5}
};

const char* kernelSource =
"__kernel void rasterCube(__global uchar4* pixels, __global float* depth, int width, int height, float t) {"
"    int x = get_global_id(0);"
"    int y = get_global_id(1);"
"    float s = sin(t), c = cos(t);"
"    float sX = sin(t*0.7f), cX = cos(t*0.7f);"
"    float3 verts[8];"
"    verts[0]= (float3)(-0.5f,-0.5f,-0.5f); verts[1]= (float3)(0.5f,-0.5f,-0.5f);"
"    verts[2]= (float3)(0.5f,0.5f,-0.5f); verts[3]= (float3)(-0.5f,0.5f,-0.5f);"
"    verts[4]= (float3)(-0.5f,-0.5f,0.5f); verts[5]= (float3)(0.5f,-0.5f,0.5f);"
"    verts[6]= (float3)(0.5f,0.5f,0.5f); verts[7]= (float3)(-0.5f,0.5f,0.5f);"
"    int tris[12][3] = {{0,1,2},{0,2,3},{4,5,6},{4,6,7},{0,1,5},{0,5,4},"
"                       {2,3,7},{2,7,6},{0,3,7},{0,7,4},{1,2,6},{1,6,5}};"
"    uchar colors_r[6] = {255,0,0,255,255,0};"
"    uchar colors_g[6] = {0,255,0,255,0,255};"
"    uchar colors_b[6] = {0,0,255,0,255,255};"
"    float3 proj[8];"
"    for(int i=0;i<8;i++){"
"        float y0 = verts[i].y*cX - verts[i].z*sX;"
"        float z0 = verts[i].y*sX + verts[i].z*cX;"
"        float x0 = verts[i].x;"
"        float x1 = x0*c - z0*s;"
"        float z1 = x0*s + z0*c;"
"        float f = 1.0f/(z1+2.0f);"
"        proj[i] = (float3)((x1*f+0.5f)*(float)width,(y0*f+0.5f)*(float)height,z1);"
"    }"
"    float mindepth = 1e10f;"
"    uchar4 color = (uchar4)(0,0,0,255);"
"    for(int i=0;i<12;i++){"
"        int faceID = i/2;"
"        uchar r = colors_r[faceID]; uchar g = colors_g[faceID]; uchar b = colors_b[faceID];"
"        float2 v0 = (float2)(proj[tris[i][0]].x, proj[tris[i][0]].y);"
"        float2 v1 = (float2)(proj[tris[i][1]].x, proj[tris[i][1]].y);"
"        float2 v2 = (float2)(proj[tris[i][2]].x, proj[tris[i][2]].y);"
"        float px = (float)x; float py = (float)y;"
"        float den = (v1.x-v0.x)*(v2.y-v0.y)-(v2.x-v0.x)*(v1.y-v0.y);"
"        if(fabs(den)<1e-6f) continue;"
"        float a = ((v2.y-v0.y)*(px-v0.x)-(v2.x-v0.x)*(py-v0.y))/den;"
"        float b_coef = (-(v1.y-v0.y)*(px-v0.x)+(v1.x-v0.x)*(py-v0.y))/den;"
"        float g_coef = 1.0f-a-b_coef;"
"        if(a>=0 && b_coef>=0 && g_coef>=0){"
"            float z = a*proj[tris[i][1]].z + b_coef*proj[tris[i][2]].z + g_coef*proj[tris[i][0]].z;"
"            if(z<mindepth){ mindepth=z; color = (uchar4)(r,g,b,255);}"
"        }"
"    }"
"    pixels[y*width+x]=color;"
"    depth[y*width+x]=mindepth;"
"}";

HBITMAP hBitmap;
HDC hMemDC;
uchar4* pixelBuffer = NULL;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
    switch(msg){
        case WM_PAINT:{
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd,&ps);
            BitBlt(hdc,0,0,WIDTH,HEIGHT,hMemDC,0,0,SRCCOPY);
            EndPaint(hwnd,&ps);
        } break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd,msg,wParam,lParam);
}

int main(){
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "OpenCLCube";
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0,"OpenCLCube","3D Cube Rasterizer",
        WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,WIDTH,HEIGHT,
        NULL,NULL,wc.hInstance,NULL);
    ShowWindow(hwnd,SW_SHOW);

    HDC hdc = GetDC(hwnd);
    hMemDC = CreateCompatibleDC(hdc);

    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = WIDTH;
    bmi.bmiHeader.biHeight = -HEIGHT;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    hBitmap = CreateDIBSection(hMemDC,&bmi,DIB_RGB_COLORS,(void**)&pixelBuffer,NULL,0);
    SelectObject(hMemDC,hBitmap);

    cl_platform_id platform;
    cl_device_id device;
    clGetPlatformIDs(1,&platform,NULL);
    clGetDeviceIDs(platform,CL_DEVICE_TYPE_GPU,1,&device,NULL);

    cl_context context = clCreateContext(NULL,1,&device,NULL,NULL,NULL);
    cl_command_queue queue = clCreateCommandQueue(context,device,0,NULL);
    cl_int err;
    cl_program program = clCreateProgramWithSource(context, 1, &kernelSource, NULL, &err);
    if(err != CL_SUCCESS) {
        printf("Error creating program: %d\n", err);
    }

    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if(err != CL_SUCCESS) {
        size_t log_size;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        char *log = (char*)malloc(log_size);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
        printf("OpenCL build error:\n%s\n", log);
        free(log);
        return 1;
    }
    cl_kernel kernel = clCreateKernel(program,"rasterCube",NULL);

    cl_mem clPixels = clCreateBuffer(context,CL_MEM_WRITE_ONLY,WIDTH*HEIGHT*sizeof(uchar4),NULL,NULL);
    cl_mem clDepth = clCreateBuffer(context,CL_MEM_WRITE_ONLY,WIDTH*HEIGHT*sizeof(float),NULL,NULL);

    MSG msg = {0};
    float t=0.0f;

    int width = WIDTH, height = HEIGHT;
    while(msg.message!=WM_QUIT){
        while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)){
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        clSetKernelArg(kernel,0,sizeof(cl_mem),&clPixels);
        clSetKernelArg(kernel,1,sizeof(cl_mem),&clDepth);
        clSetKernelArg(kernel,2,sizeof(int),&width);
        clSetKernelArg(kernel,3,sizeof(int),&height);
        clSetKernelArg(kernel,4,sizeof(float),&t);

        size_t global[2]={WIDTH,HEIGHT};
        clEnqueueNDRangeKernel(queue,kernel,2,NULL,global,NULL,0,NULL,NULL);
        clEnqueueReadBuffer(queue,clPixels,CL_TRUE,0,WIDTH*HEIGHT*sizeof(uchar4),pixelBuffer,0,NULL,NULL);

        InvalidateRect(hwnd,NULL,FALSE);
        t+=0.0002f;

        Sleep(16);
    }

    clReleaseMemObject(clPixels);
    clReleaseMemObject(clDepth);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    DeleteObject(hBitmap);
    DeleteDC(hMemDC);
    ReleaseDC(hwnd,hdc);

    return 0;
}

#pragma once

#include "gab_math.h"
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

typedef struct {
    f3 vertex[3]; 
    f2 uv[3];
    f3 normal[3];
    Color color;
} Triangle;

typedef struct {
    Triangle* triangles;   
    size_t numTriangles;
    size_t numVertices;
    int texWidth;
    int texHeight;
    Color* pixels;
    f4x4 transform;
} CustomModel;

typedef struct {
    int triangleOffset;
    int triangleCount;
    int vertexOffset;
    int vertexCount;
    int pixelOffset;
    int texWidth;
    int texHeight;
    f4x4 transform;
} CustomModelGPU;

typedef enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
} Camera_Movement;

typedef struct 
{
  f3 Position;
  f3 Front;
  f3 Up;
  f3 Right;
  f3 WorldUp;
  f4x4 proj;
  f4x4 look_at;
  float near_plane;
  float far_plane;
  float fov;
  float fov_rad;
  float aspect_ratio;
  float yaw;
  float pitch;
  float speed;
  float sens;
  float lastX;
  float lastY;
  bool firstMouse;
  float deltaTime;

} CustomCamera;

void engine_init(Engine* engine,CustomCamera* camera,
                 CustomModel* model,const char* kernel,
                 int width, int height);

void engine_background_color(Engine* engine, Color color);


void engine_clear_background(Engine* engine);


void engine_send_camera_matrix(Engine* engine, CustomCamera* camera);


void engine_run_rasterizer(Engine* engine, CustomModel* model);


void engine_read_and_display(Engine* engine);


void engine_close(Engine* engine);


void engine_load_model(CustomModel* model, const char* filePath,const char* texturePath, Color color);


void engine_set_model_transform(CustomModel* model,f4x4 transform);


void engine_print_model_data(const CustomModel* model);


void engine_free_model(CustomModel* model);


void engine_init_camera(CustomCamera* camera, int width, int height, float fov,
                        float near_plane, float far_plane);

void engine_process_camera_keys(CustomCamera* camera, Camera_Movement direction);


void engine_update_camera(CustomCamera* camera, float mouseX, float mouseY, bool constrainPitch);




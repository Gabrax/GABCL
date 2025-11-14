#pragma once

#include "gab_math.h"
#include <CL/cl.h>
#include <stdlib.h>
#include <stdio.h>
#include <raylib.h>

typedef struct {
    f3 vertex[3]; 
    f2 uv[3];
    f3 normal[3];
    Color color;
} Triangle;

typedef enum {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
} Movement;

void engine_init(const char* kernel,int width, int height);
void engine_background_color(Color color);
void engine_clear_background();
void engine_send_camera_matrix();
void engine_run_rasterizer();
void engine_read_and_display();
void engine_close();

void engine_load_model(const char* filePath,const char* texturePath,f4x4 transform);
void engine_upload_models_data();
void engine_print_model_data();
void engine_free_model();
void engine_init_camera(int width, int height, float fov, float near_plane, float far_plane);

void engine_process_camera_keys(Movement direction);
void engine_update_camera(float mouseX, float mouseY, bool constrainPitch);




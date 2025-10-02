#pragma once
#include "algebra.h"
#define _USE_MATH_DEFINES
#include <math.h>

typedef struct
{
  Vec3F position, target, direction, right_axis, up;
  Mat4F projection;
  float aspect_ratio, fov, fov_radians, far_plane, near_plane;

} Camera;

void camera_init(Camera* camera, Vec3F position, int render_width, int render_height, float fov, float near_plane, float far_plane)
{
  camera->position = position;
  camera->aspect_ratio = (float)(render_height) / (float)(render_width); 
  camera->near_plane = near_plane;
  camera->far_plane = far_plane;
  camera->fov = fov;
  camera->fov_radians = 1.0f / tanf(camera->fov * 0.5f / 180.0f * M_PI);

}

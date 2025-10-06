#pragma once

#include "algebra.h"

typedef struct 
{
  Vec3 Position;
  Vec3 Front;
  Vec3 Up;
  Vec3 Right;
  Vec3 WorldUp; 
  float near_plane;
  float far_plane;
  float fov;
  float fov_rad;
  float aspect_ratio;
  float yaw;
  float pitch;
  float speed;
  float sens;

} GAB_Camera;


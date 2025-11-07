#pragma once

#define GABMATH_IMPLEMENTATION
#include "gab_math.h"

#include <stdbool.h>

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

} GAB_Camera;

void ProcessKeyboard(GAB_Camera* camera, Camera_Movement direction, float deltaTime)
{
  float velocity = camera->speed * deltaTime;

  if (direction == FORWARD) camera->Position = f3Add(camera->Position, f3MulS(camera->Front, velocity));
  if (direction == BACKWARD) camera->Position = f3Add(camera->Position, f3MulS(camera->Front, -velocity));
  if (direction == LEFT) camera->Position = f3Add(camera->Position, f3MulS(camera->Right, velocity));
  if (direction == RIGHT) camera->Position = f3Add(camera->Position, f3MulS(camera->Right, -velocity));
}
void updateCamera(GAB_Camera* camera, float mouseX, float mouseY, bool constrainPitch)
{
  float xoffset, yoffset;
  if (camera->firstMouse)
  {
      camera->lastX = mouseX;
      camera->lastY = mouseY;
      camera->firstMouse = false;
  }

  xoffset = camera->lastX - mouseX;
  yoffset = camera->lastY - mouseY; 

  camera->lastX = mouseX;
  camera->lastY = mouseY;

  xoffset *= camera->sens;
  yoffset *= camera->sens;

  camera->yaw   += xoffset;
  camera->pitch += yoffset;

  if (constrainPitch)
  {
      if (camera->pitch > 89.0f)  camera->pitch = 89.0f;
      if (camera->pitch < -89.0f) camera->pitch = -89.0f;
  }

  f3 front;
  front.x = cosf(DegToRad(camera->yaw)) * cosf(DegToRad(camera->pitch));
  front.y = sinf(DegToRad(camera->pitch));
  front.z = sinf(DegToRad(camera->yaw)) * cosf(DegToRad(camera->pitch));
  camera->Front = f3Norm(front);

  camera->Right = f3Norm(f3Cross(camera->Front, camera->WorldUp));
  camera->Up    = f3Norm(f3Cross(camera->Right, camera->Front));

  camera->look_at = MatLookAt(camera->Position, f3Add(camera->Position, camera->Front), camera->Up);
}

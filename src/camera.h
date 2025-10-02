#pragma once

#include "algebra.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "window.h"

typedef struct
{
  Vec3F position, front, up;
  Mat4F view_projection;
  float aspect_ratio, fov, far_plane, near_plane;
  float lastX, lastY, pitch, yaw;

} Camera;

inline void camera_init(Camera* camera, Vec3F position, int render_width, int render_height, float fov, float near_plane, float far_plane)
{
  camera->position = position;
  camera->aspect_ratio = (float)(render_width) / (float)(render_height); 
  camera->near_plane = near_plane;
  camera->far_plane = far_plane;
  camera->fov = fov;
  camera->front = (Vec3F){0.0,0.0,-1.0};
  camera->up = (Vec3F){0.0,1.0,0.0};

  Mat4F view = mat4_look_at(camera->position,vec3_add(camera->position, camera->front),camera->up);
  Mat4F projection = mat4_perspective(camera->fov, camera->aspect_ratio, camera->near_plane, camera->far_plane);

  camera->view_projection = mat4_mul(projection, view);
}

inline void camera_update(Window* window, Camera* camera, float movement_speed, float sensitivity)
{
  if(GetAsyncKeyState('W') & 0x8000) camera->position = vec3_add(camera->position, vec3_scale(camera->front, movement_speed));
  if(GetAsyncKeyState('S') & 0x8000) camera->position = vec3_sub(camera->position, vec3_scale(camera->front, movement_speed));
  if(GetAsyncKeyState('A') & 0x8000) camera->position = vec3_sub(camera->position, vec3_scale(vec3_normalize(vec3_cross(camera->front, camera->up)), movement_speed));
  if(GetAsyncKeyState('D') & 0x8000) camera->position = vec3_add(camera->position, vec3_scale(vec3_normalize(vec3_cross(camera->front, camera->up)), movement_speed));

  POINT cursorPos;
  GetCursorPos(&cursorPos);               // get absolute cursor position
  ScreenToClient(window->hwnd, &cursorPos); // convert to window client coordinates

  static int lastX = -1, lastY = -1;
  if(lastX == -1 && lastY == -1) { lastX = cursorPos.x; lastY = cursorPos.y; }

  int dx = cursorPos.x - lastX;
  int dy = lastY - cursorPos.y; // reversed: y-coordinates

  lastX = cursorPos.x;
  lastY = cursorPos.y;

  float yaw   = dx * sensitivity;
  float pitch = dy * sensitivity;

  // update camera direction
  camera->yaw   += yaw;
  camera->pitch += pitch;

  if(camera->pitch > 89.0f) camera->pitch = 89.0f;
  if(camera->pitch < -89.0f) camera->pitch = -89.0f;

  Vec3F front;
  front.x = cosf(deg2rad(camera->yaw)) * cosf(deg2rad(camera->pitch));
  front.y = sinf(deg2rad(camera->pitch));
  front.z = sinf(deg2rad(camera->yaw)) * cosf(deg2rad(camera->pitch));
  camera->front = vec3_normalize(front);

  // --- update view-projection matrix ---
  Mat4F view = mat4_look_at(camera->position, vec3_add(camera->position, camera->front), camera->up);
  Mat4F projection = mat4_perspective(camera->fov, camera->aspect_ratio, camera->near_plane, camera->far_plane);
  camera->view_projection = mat4_mul(projection, view);
}

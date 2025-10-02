#include "algebra.h"
#define _USE_MATH_DEFINES
#include <math.h>

Vec3F vec3_reverse(Vec3F vec)
{
  return (Vec3F){-vec.x, -vec.y, -vec.z};
}
Vec3F vec3_add(Vec3F vec1, Vec3F vec2)
{
  Vec3F res;
  res.x = vec1.x + vec2.x;
  res.y = vec1.y + vec2.y;
  res.z = vec1.z + vec2.z;
  return res;
}
Vec3F vec3_sub(Vec3F vec1, Vec3F vec2)
{
  Vec3F res;
  res.x = vec1.x - vec2.x;
  res.y = vec1.y - vec2.y;
  res.z = vec1.z - vec2.z;
  return res;
}
float vec3_dot(Vec3F vec1, Vec3F vec2)
{
  return (vec1.x * vec2.x) + (vec1.y * vec2.y) + (vec1.z * vec2.z);
}
Vec3F vec3_cross_product(Vec3F vec1, Vec3F vec2)
{
  Vec3F res;
  res.x = vec1.y * vec2.z - vec1.z * vec2.y;
  res.y = vec1.z * vec2.x - vec1.x * vec2.z;
  res.z = vec1.x * vec2.y - vec1.y * vec2.x;
  return res;
}
Vec3F vec3_normalize(Vec3F vec)
{
  int len = sqrtf(powf(vec.x, 2) + powf(vec.y, 2));
  if (len == 0.0f) return (Vec3F){0,0,0}; 
  return (Vec3F){ vec.x/len, vec.y/len, vec.z/len };
}

Mat4F mat4_identity()
{
  Mat4F m = {
      1,0,0,0,
      0,1,0,0,
      0,0,1,0,
      0,0,0,1
  };
  return m;
}
void mat4_translate(Mat4F* mat, Vec3F position)
{
  mat->x4 += position.x;
  mat->y4 += position.y;
  mat->z4 += position.z;
}
void mat4_scale(Mat4F* mat, Vec3F scale)
{
  mat->x1 = scale.x;
  mat->y2 = scale.y;
  mat->z3 = scale.z;
}
void mat4_rotate(Mat4F* mat, float degrees, Vec3F axis)
{
  float radians = deg2rad(degrees);
  float c = cosf(radians);
  float s = sinf(radians);
  float one_c = 1.0f - c;

  // normalize axis
  float len = sqrtf(axis.x*axis.x + axis.y*axis.y + axis.z*axis.z);
  float x = axis.x / len;
  float y = axis.y / len;
  float z = axis.z / len;

  Mat4F R = {
      .x1 = c + x*x*one_c,     .x2 = x*y*one_c - z*s, .x3 = x*z*one_c + y*s, .x4 = 0,
      .y1 = y*x*one_c + z*s,   .y2 = c + y*y*one_c,   .y3 = y*z*one_c - x*s, .y4 = 0,
      .z1 = z*x*one_c - y*s,   .z2 = z*y*one_c + x*s, .z3 = c + z*z*one_c,   .z4 = 0,
      .w1 = 0,                 .w2 = 0,               .w3 = 0,               .w4 = 1
  };

  *mat = mat4_mul(R, *mat);
}
void mat4_rotate_x(Mat4F* mat, float degrees)
{
  float radians = deg2rad(degrees);
  float c = cosf(radians);
  float s = sinf(radians);

  Mat4F Rx = {
      .x1 = 1, .x2 = 0,  .x3 = 0,  .x4 = 0,
      .y1 = 0, .y2 = c,  .y3 = -s, .y4 = 0,
      .z1 = 0, .z2 = s,  .z3 = c,  .z4 = 0,
      .w1 = 0, .w2 = 0,  .w3 = 0,  .w4 = 1
  };

  *mat = mat4_mul(Rx, *mat);
}
void mat4_rotate_y(Mat4F* mat, float degrees)
{
  float radians = deg2rad(degrees);
  float c = cosf(radians);
  float s = sinf(radians);

  Mat4F Ry = {
      .x1 = c,  .x2 = 0, .x3 = s,  .x4 = 0,
      .y1 = 0,  .y2 = 1, .y3 = 0,  .y4 = 0,
      .z1 = -s, .z2 = 0, .z3 = c,  .z4 = 0,
      .w1 = 0,  .w2 = 0, .w3 = 0,  .w4 = 1
  };

  *mat = mat4_mul(Ry, *mat);
}
void mat4_rotate_z(Mat4F* mat, float degrees)
{
  float radians = deg2rad(degrees);
  float c = cosf(radians);
  float s = sinf(radians);

  Mat4F Rz = {
      .x1 = c,  .x2 = -s, .x3 = 0, .x4 = 0,
      .y1 = s,  .y2 = c,  .y3 = 0, .y4 = 0,
      .z1 = 0,  .z2 = 0,  .z3 = 1, .z4 = 0,
      .w1 = 0,  .w2 = 0,  .w3 = 0, .w4 = 1
  };

  *mat = mat4_mul(Rz, *mat);
}
Mat4F mat4_mul(Mat4F a, Mat4F b)
{
  Mat4F r;
  r.x1 = a.x1*b.x1 + a.x2*b.y1 + a.x3*b.z1 + a.x4*b.w1;
  r.x2 = a.x1*b.x2 + a.x2*b.y2 + a.x3*b.z2 + a.x4*b.w2;
  r.x3 = a.x1*b.x3 + a.x2*b.y3 + a.x3*b.z3 + a.x4*b.w3;
  r.x4 = a.x1*b.x4 + a.x2*b.y4 + a.x3*b.z4 + a.x4*b.w4;

  r.y1 = a.y1*b.x1 + a.y2*b.y1 + a.y3*b.z1 + a.y4*b.w1;
  r.y2 = a.y1*b.x2 + a.y2*b.y2 + a.y3*b.z2 + a.y4*b.w2;
  r.y3 = a.y1*b.x3 + a.y2*b.y3 + a.y3*b.z3 + a.y4*b.w3;
  r.y4 = a.y1*b.x4 + a.y2*b.y4 + a.y3*b.z4 + a.y4*b.w4;

  r.z1 = a.z1*b.x1 + a.z2*b.y1 + a.z3*b.z1 + a.z4*b.w1;
  r.z2 = a.z1*b.x2 + a.z2*b.y2 + a.z3*b.z2 + a.z4*b.w2;
  r.z3 = a.z1*b.x3 + a.z2*b.y3 + a.z3*b.z3 + a.z4*b.w3;
  r.z4 = a.z1*b.x4 + a.z2*b.y4 + a.z3*b.z4 + a.z4*b.w4;

  r.w1 = a.w1*b.x1 + a.w2*b.y1 + a.w3*b.z1 + a.w4*b.w1;
  r.w2 = a.w1*b.x2 + a.w2*b.y2 + a.w3*b.z2 + a.w4*b.w2;
  r.w3 = a.w1*b.x3 + a.w2*b.y3 + a.w3*b.z3 + a.w4*b.w3;
  r.w4 = a.w1*b.x4 + a.w2*b.y4 + a.w3*b.z4 + a.w4*b.w4;
  return r;
}
float mat4_det(Mat4F m)
{
  float det =
      m.x1 * (m.y2*(m.z3*m.w4 - m.z4*m.w3) - m.y3*(m.z2*m.w4 - m.z4*m.w2) + m.y4*(m.z2*m.w3 - m.z3*m.w2)) -
      m.x2 * (m.y1*(m.z3*m.w4 - m.z4*m.w3) - m.y3*(m.z1*m.w4 - m.z4*m.w1) + m.y4*(m.z1*m.w3 - m.z3*m.w1)) +
      m.x3 * (m.y1*(m.z2*m.w4 - m.z4*m.w2) - m.y2*(m.z1*m.w4 - m.z4*m.w1) + m.y4*(m.z1*m.w2 - m.z2*m.w1)) -
      m.x4 * (m.y1*(m.z2*m.w3 - m.z3*m.w2) - m.y2*(m.z1*m.w3 - m.z3*m.w1) + m.y3*(m.z1*m.w2 - m.z2*m.w1));
  return det;
}
Mat4F mat4_inverse_affine(Mat4F m)
{
  float det = m.x1*m.y2 - m.x2*m.y1;
  if (fabs(det)<1e-6) return mat4_identity();
  float invDet = 1.0f/det;
  Mat4F inv = mat4_identity();
  inv.x1 =  m.y2*invDet;
  inv.x2 = -m.x2*invDet;
  inv.y1 = -m.y1*invDet;
  inv.y2 =  m.x1*invDet;
  inv.x4 = -(inv.x1*m.x4 + inv.x2*m.y4);
  inv.y4 = -(inv.y1*m.x4 + inv.y2*m.y4);
  return inv;
}

float deg2rad(float degrees) {
    return degrees * M_PI / 180.0;
}


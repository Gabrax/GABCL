#pragma once

#include <math.h>
#include <stdio.h>

inline float DegToRad(float degrees)
{
  return degrees * (3.14159265358979323846f / 180.0f);
}

typedef struct Vec2 { float x,y; } Vec2;
typedef struct Vec3 { float x,y,z; } Vec3;
inline void Vec2Print(Vec2* v)
{
  printf("[ %f %f ]\n", v->x,v->y);
}
inline void Vec3Print(Vec3* v)
{
  printf("[ %f %f %f ]\n", v->x,v->y,v->z);
}

inline Vec3 Vec3Add(Vec3 v1, Vec3 v2)
{
  return (Vec3){v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
}
inline Vec3 Vec3Sub(Vec3 v1, Vec3 v2)
{
  return (Vec3){v2.x - v1.x, v2.y - v1.y, v2.z - v1.z};
}
inline Vec3 Vec3Multiply(Vec3 v1, Vec3 v2)
{
  return (Vec3){ v1.x * v2.x, v1.y * v2.y, v1.z * v2.z };
}
inline Vec3 Vec3MultiplyScalar(Vec3 v, float scalar)
{
  return (Vec3){ v.x * scalar, v.y * scalar, v.z * scalar };
}
inline Vec3 Vec3Cross(Vec3 v1, Vec3 v2) // Returns orthogonal vector
{
  return (Vec3){v1.y * v2.z - v1.z * v2.y,v1.z * v2.x - v1.x * v2.z,v1.x * v2.y - v1.y * v2.x};
}
inline float Vec3Dot(Vec3 v1, Vec3 v2)
{
  return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
inline float Vec3Len(Vec3 v)
{
  return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}
inline float Vec3Theta(Vec3 v1, Vec3 v2) // returns angle in radians between vectors (reverse cos)
{
  return acosf(Vec3Dot(v1, v2) / (Vec3Len(v1) * Vec3Len(v2)));
}
inline Vec3 Vec3Norm(Vec3 v)
{
  float len = Vec3Len(v);
  if (len > 0.0f) return (Vec3){ v.x / len, v.y / len, v.z / len };
  else return (Vec3){ 0.0f, 0.0f, 0.0f }; 
}
// COLUMN-MAJOR
typedef struct Mat4 {
    float x0, x1, x2, x3;
    float y0, y1, y2, y3;
    float z0, z1, z2, z3;
    float w0, w1, w2, w3;
} Mat4;
inline void MatPrint(Mat4* mat)
{
  printf("[ %f %f %f %f ]\n", mat->x0,mat->x1,mat->x2,mat->x3);
  printf("[ %f %f %f %f ]\n", mat->y0,mat->y1,mat->y2,mat->y3);
  printf("[ %f %f %f %f ]\n", mat->z0,mat->z1,mat->z2,mat->z3);
  printf("[ %f %f %f %f ]\n", mat->w0,mat->w1,mat->w2,mat->w3);
}
inline Mat4 MatMul(Mat4 a, Mat4 b)
{
  Mat4 result = {0};

  // Row 0
  result.x0 = a.x0*b.x0 + a.x1*b.y0 + a.x2*b.z0 + a.x3*b.w0;
  result.x1 = a.x0*b.x1 + a.x1*b.y1 + a.x2*b.z1 + a.x3*b.w1;
  result.x2 = a.x0*b.x2 + a.x1*b.y2 + a.x2*b.z2 + a.x3*b.w2;
  result.x3 = a.x0*b.x3 + a.x1*b.y3 + a.x2*b.z3 + a.x3*b.w3;

  // Row 1
  result.y0 = a.y0*b.x0 + a.y1*b.y0 + a.y2*b.z0 + a.y3*b.w0;
  result.y1 = a.y0*b.x1 + a.y1*b.y1 + a.y2*b.z1 + a.y3*b.w1;
  result.y2 = a.y0*b.x2 + a.y1*b.y2 + a.y2*b.z2 + a.y3*b.w2;
  result.y3 = a.y0*b.x3 + a.y1*b.y3 + a.y2*b.z3 + a.y3*b.w3;

  // Row 2
  result.z0 = a.z0*b.x0 + a.z1*b.y0 + a.z2*b.z0 + a.z3*b.w0;
  result.z1 = a.z0*b.x1 + a.z1*b.y1 + a.z2*b.z1 + a.z3*b.w1;
  result.z2 = a.z0*b.x2 + a.z1*b.y2 + a.z2*b.z2 + a.z3*b.w2;
  result.z3 = a.z0*b.x3 + a.z1*b.y3 + a.z2*b.z3 + a.z3*b.w3;

  // Row 3
  result.w0 = a.w0*b.x0 + a.w1*b.y0 + a.w2*b.z0 + a.w3*b.w0;
  result.w1 = a.w0*b.x1 + a.w1*b.y1 + a.w2*b.z1 + a.w3*b.w1;
  result.w2 = a.w0*b.x2 + a.w1*b.y2 + a.w2*b.z2 + a.w3*b.w2;
  result.w3 = a.w0*b.x3 + a.w1*b.y3 + a.w2*b.z3 + a.w3*b.w3;

  return result;
}
inline Mat4 MatIdentity()
{
  Mat4 mat = { 0 };
  mat.x0  = 1.0f;  
  mat.y1  = 1.0f;
  mat.z2 = 1.0f;
  mat.w3 = 1.0f;
  return mat;
}
inline Mat4 MatPerspective(float fov_rad, float aspect_ratio, float near_plane, float far_plane)
{
  Mat4 mat = { 0 };  
  mat.x0  = fov_rad / aspect_ratio;      // 1/(tan(fov/2)*aspect)
  mat.y1  = fov_rad;          // 1/tan(fov/2)
  mat.z2 = -(far_plane + near_plane) / (far_plane - near_plane);
  mat.w2 = -1.0f;
  mat.z3 = -(2.0f * far_plane * near_plane) / (far_plane - near_plane);
  mat.w3 = 0.0f;
  return mat;
};
inline Mat4 MatTranslate(const Mat4 mat, Vec3 position)
{
  Mat4 trans = MatIdentity();
  trans.x3 = position.x;
  trans.y3 = position.y;
  trans.z3 = position.z;
  return MatMul(mat, trans);
}
inline Mat4 MatRotateX(const Mat4 mat, float radians)
{
  Mat4 rot = MatIdentity();
  rot.y1 = cosf(radians); rot.y2 = -sinf(radians);
  rot.z1 = sinf(radians); rot.z2 = cosf(radians);
  return MatMul(mat, rot);
}
inline Mat4 MatRotateY(const Mat4 mat, float radians)
{
  Mat4 rot = MatIdentity();
  rot.x0 = cosf(radians);  rot.x2 = sinf(radians);
  rot.z0 = -sinf(radians); rot.z2 = cosf(radians);
  return MatMul(mat, rot);
}
inline Mat4 MatRotateZ(const Mat4 mat, float radians)
{
  Mat4 rot = MatIdentity();
  rot.x0 = cosf(radians); rot.x1 = -sinf(radians);
  rot.y0 = sinf(radians); rot.y1 = cosf(radians);
  return MatMul(mat, rot);
}
inline Mat4 MatScale(const Mat4 mat, Vec3 scale)
{
  Mat4 s = MatIdentity();
  s.x0 = scale.x;
  s.y1 = scale.y;
  s.z2 = scale.z;
  return MatMul(mat, s);
}
inline Mat4 MatTransform(Vec3 position, Vec3 deg_rotation, Vec3 scale)
{
  Mat4 mat = MatIdentity();

  mat = MatTranslate(mat, position);

  mat = MatScale(mat, scale);

  mat = MatRotateX(mat, DegToRad(deg_rotation.x));
  mat = MatRotateY(mat, DegToRad(deg_rotation.y));
  mat = MatRotateZ(mat, DegToRad(deg_rotation.z));

  return mat;
}
inline Mat4 MatInverseRT(const Mat4* m)
{
  Mat4 inv = {0};

  // transpose rotation part
  inv.x0 = m->x0; inv.x1 = m->y0; inv.x2 = m->z0;
  inv.y0 = m->x1; inv.y1 = m->y1; inv.y2 = m->z1;
  inv.z0 = m->x2; inv.z1 = m->y2; inv.z2 = m->z2;

  // inverse translation
  inv.x3 = -(inv.x0 * m->x3 + inv.x1 * m->y3 + inv.x2 * m->z3);
  inv.y3 = -(inv.y0 * m->x3 + inv.y1 * m->y3 + inv.y2 * m->z3);
  inv.z3 = -(inv.z0 * m->x3 + inv.z1 * m->y3 + inv.z2 * m->z3);

  // bottom row
  inv.w0 = 0.0f; inv.w1 = 0.0f; inv.w2 = 0.0f; inv.w3 = 1.0f;

  return inv;
}
inline Mat4 MatLookAt(Vec3 position, Vec3 target, Vec3 up)
{
  Vec3 f = Vec3Norm(Vec3Sub(target, position));  // forward
  Vec3 s = Vec3Norm(Vec3Cross(up, f));           // right (swapped order)
  Vec3 u = Vec3Cross(f, s);                      // true up

  Mat4 mat = MatIdentity();

  // Rotation part (basis vectors)
  mat.x0 = s.x;  mat.x1 = s.y;  mat.x2 = s.z;  mat.x3 = -Vec3Dot(s, position);
  mat.y0 = u.x;  mat.y1 = u.y;  mat.y2 = u.z;  mat.y3 = -Vec3Dot(u, position);
  mat.z0 = -f.x; mat.z1 = -f.y; mat.z2 = -f.z; mat.z3 =  Vec3Dot(f, position);
  mat.w0 = 0.0f; mat.w1 = 0.0f; mat.w2 = 0.0f; mat.w3 = 1.0f;

  return mat;
}




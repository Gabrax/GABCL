#include "algebra.h"

#include <math.h>
#include <stdio.h>

float DegToRad(float degrees)
{
  return degrees * (3.14159265358979323846f / 180.0f);
}

void Vec2Print(float2* v)
{
  printf("[ %f %f ]\n", v->x,v->y);
}
void Vec3Print(float3* v)
{
  printf("[ %f %f %f ]\n", v->x,v->y,v->z);
}

float3 Vec3Add(float3 v1, float3 v2)
{
  return (float3){v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
}
float3 Vec3Sub(float3 v1, float3 v2)
{
  return (float3){v2.x - v1.x, v2.y - v1.y, v2.z - v1.z};
}
float3 Vec3Multiply(float3 v1, float3 v2)
{
  return (float3){ v1.x * v2.x, v1.y * v2.y, v1.z * v2.z };
}
float3 Vec3MultiplyScalar(float3 v, float scalar)
{
  return (float3){ v.x * scalar, v.y * scalar, v.z * scalar };
}
float3 Vec3Cross(float3 v1, float3 v2) // Returns orthogonal vector
{
  return (float3){v1.y * v2.z - v1.z * v2.y,v1.z * v2.x - v1.x * v2.z,v1.x * v2.y - v1.y * v2.x};
}
float Vec3Dot(float3 v1, float3 v2)
{
  return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
float Vec3Len(float3 v)
{
  return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}
float Vec3Theta(float3 v1, float3 v2) // returns angle in radians between vectors (reverse cos)
{
  return acosf(Vec3Dot(v1, v2) / (Vec3Len(v1) * Vec3Len(v2)));
}
float3 Vec3Norm(float3 v)
{
  float len = Vec3Len(v);
  return (float3){ v.x / len, v.y / len, v.z / len };
}

void MatPrint(float4x4* mat)
{
  printf("[ %f %f %f %f ]\n", mat->x0,mat->x1,mat->x2,mat->x3);
  printf("[ %f %f %f %f ]\n", mat->y0,mat->y1,mat->y2,mat->y3);
  printf("[ %f %f %f %f ]\n", mat->z0,mat->z1,mat->z2,mat->z3);
  printf("[ %f %f %f %f ]\n", mat->w0,mat->w1,mat->w2,mat->w3);
}
float4x4 MatMul(float4x4 a, float4x4 b)
{
  float4x4 result = {0};

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
float4x4 MatIdentity()
{
  float4x4 mat = { 0 };
  mat.x0  = 1.0f;  
  mat.y1  = 1.0f;
  mat.z2 = 1.0f;
  mat.w3 = 1.0f;
  return mat;
}
float4x4 MatPerspective(float fov_rad, float aspect_ratio, float near_plane, float far_plane)
{
  float4x4 mat = { 0 };  
  mat.x0  = fov_rad / aspect_ratio;      // 1/(tan(fov/2)*aspect)
  mat.y1  = fov_rad;          // 1/tan(fov/2)
  mat.z2 = -(far_plane + near_plane) / (far_plane - near_plane);
  mat.w2 = -1.0f;
  mat.z3 = -(2.0f * far_plane * near_plane) / (far_plane - near_plane);
  mat.w3 = 0.0f;
  return mat;
};
float4x4 MatTranslate(const float4x4 mat, float3 position)
{
  float4x4 trans = MatIdentity();
  trans.x3 = position.x;
  trans.y3 = position.y;
  trans.z3 = position.z;
  return MatMul(mat, trans);
}
float4x4 MatRotateX(const float4x4 mat, float radians)
{
  float4x4 rot = MatIdentity();
  rot.y1 = cosf(radians); rot.y2 = -sinf(radians);
  rot.z1 = sinf(radians); rot.z2 = cosf(radians);
  return MatMul(mat, rot);
}
float4x4 MatRotateY(const float4x4 mat, float radians)
{
  float4x4 rot = MatIdentity();
  rot.x0 = cosf(radians);  rot.x2 = sinf(radians);
  rot.z0 = -sinf(radians); rot.z2 = cosf(radians);
  return MatMul(mat, rot);
}
float4x4 MatRotateZ(const float4x4 mat, float radians)
{
  float4x4 rot = MatIdentity();
  rot.x0 = cosf(radians); rot.x1 = -sinf(radians);
  rot.y0 = sinf(radians); rot.y1 = cosf(radians);
  return MatMul(mat, rot);
}
float4x4 MatScale(const float4x4 mat, float3 scale)
{
  float4x4 s = MatIdentity();
  s.x0 = scale.x;
  s.y1 = scale.y;
  s.z2 = scale.z;
  return MatMul(mat, s);
}
float4x4 MatTransform(float3 position, float3 deg_rotation, float3 scale)
{
  float4x4 mat = MatIdentity();

  mat = MatTranslate(mat, position);

  mat = MatScale(mat, scale);

  mat = MatRotateX(mat, DegToRad(deg_rotation.x));
  mat = MatRotateY(mat, DegToRad(deg_rotation.y));
  mat = MatRotateZ(mat, DegToRad(deg_rotation.z));

  return mat;
}
float4x4 MatInverseRT(const float4x4* m)
{
  float4x4 inv = {0};

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
float4x4 MatLookAt(float3 position, float3 target, float3 up)
{
  float3 f = Vec3Norm(Vec3Sub(target, position));  // forward
  float3 s = Vec3Norm(Vec3Cross(up, f));           // right (swapped order)
  float3 u = Vec3Cross(f, s);                      // true up

  float4x4 mat = MatIdentity();

  // Rotation part (basis vectors)
  mat.x0 = s.x;  mat.x1 = s.y;  mat.x2 = s.z;  mat.x3 = -Vec3Dot(s, position);
  mat.y0 = u.x;  mat.y1 = u.y;  mat.y2 = u.z;  mat.y3 = -Vec3Dot(u, position);
  mat.z0 = -f.x; mat.z1 = -f.y; mat.z2 = -f.z; mat.z3 =  Vec3Dot(f, position);
  mat.w0 = 0.0f; mat.w1 = 0.0f; mat.w2 = 0.0f; mat.w3 = 1.0f;

  return mat;
}

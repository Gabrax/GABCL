#pragma once

float DegToRad(float degrees);

typedef struct float2 { float x,y; } float2;
typedef struct float3 { float x,y,z; } float3;
typedef struct float4 { float x,y,z,w; } float4;

void Vec2Print(float2* v);
void Vec3Print(float3* v);

float3 Vec3Add(float3 v1, float3 v2);
float3 Vec3Sub(float3 v1, float3 v2);
float3 Vec3Multiply(float3 v1, float3 v2);
float3 Vec3MultiplyScalar(float3 v, float scalar);
float3 Vec3Cross(float3 v1, float3 v2); // Returns orthogonal vector
float Vec3Dot(float3 v1, float3 v2);
float Vec3Len(float3 v);
float Vec3Theta(float3 v1, float3 v2); // returns angle in radians between vectors (reverse cos)
float3 Vec3Norm(float3 v);
// COLUMN-MAJOR
typedef struct float4x4 {
    float x0, x1, x2, x3;
    float y0, y1, y2, y3;
    float z0, z1, z2, z3;
    float w0, w1, w2, w3;
} float4x4;

void MatPrint(float4x4* mat);
float4x4 MatMul(float4x4 a, float4x4 b);
float4x4 MatIdentity();
float4x4 MatPerspective(float fov_rad, float aspect_ratio, float near_plane, float far_plane);
float4x4 MatTranslate(const float4x4 mat, float3 position);
float4x4 MatRotateX(const float4x4 mat, float radians);
float4x4 MatRotateY(const float4x4 mat, float radians);
float4x4 MatRotateZ(const float4x4 mat, float radians);
float4x4 MatScale(const float4x4 mat, float3 scale);
float4x4 MatTransform(float3 position, float3 deg_rotation, float3 scale);
float4x4 MatInverseRT(const float4x4* m);
float4x4 MatLookAt(float3 position, float3 target, float3 up);




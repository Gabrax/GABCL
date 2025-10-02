#pragma once

typedef struct {
    float x, y;
} Vec2F;

typedef struct {
    float x, y, z;
} Vec3F;

Vec3F vec3_reverse(Vec3F vec);
Vec3F vec3_add(Vec3F vec1, Vec3F vec2);
Vec3F vec3_sub(Vec3F vec1, Vec3F vec2);
float vec3_dot(Vec3F vec1, Vec3F vec2);
Vec3F vec3_cross(Vec3F vec1, Vec3F vec2);
Vec3F vec3_normalize(Vec3F vec);

typedef struct {
    float x1, y1, z1;
    float x2, y2, z2;
    float x3, y3, z3;
} Mat3F;

typedef struct {
    float x1, y1, z1, w1;
    float x2, y2, z2, w2;
    float x3, y3, z3, w3;
    float x4, y4, z4, w4;
} Mat4F;

Mat4F mat4_identity();
void mat4_translate(Mat4F* mat, Vec3F position);
void mat4_scale(Mat4F* mat, Vec3F scale);
void mat4_rotate(Mat4F* mat, float degrees, Vec3F rotation);
void mat4_rotate_x(Mat4F* mat, float degrees);
void mat4_rotate_y(Mat4F* mat, float degrees);
void mat4_rotate_z(Mat4F* mat, float degrees);
Mat4F mat4_mul(Mat4F mat1, Mat4F mat2);
float mat4_det(Mat4F mat);
Mat4F mat4_inverse_affine(Mat4F mat);
Mat4F look_at(Vec3F position, Vec3F target, Vec3F up);

float deg2rad(float degrees);

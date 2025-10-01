#ifndef MATH_H 
#define MATH_H

typedef struct {
    float x, y;
} Vec2F;

typedef struct {
    float x, y, z;
} Vec3F;

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
Mat4F mat4_translate(float tx, float ty, float tz);
Mat4F mat4_scale(float sx, float sy, float sz);
Mat4F mat4_rotate_x(float radians);
Mat4F mat4_rotate_y(float radians);
Mat4F mat4_rotate_z(float radians);

#endif // MATH_H 

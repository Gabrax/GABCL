#include "math.h"
#include <math.h>

Mat4F mat4_identity() {
    return (Mat4F){
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    };
}

Mat4F mat4_translate(float tx, float ty, float tz) {
    Mat4F m = mat4_identity();
    m.x4 = tx;
    m.y4 = ty;
    m.z4 = tz;
    return m;
}

Mat4F mat4_scale(float sx, float sy, float sz) {
    Mat4F m = mat4_identity();
    m.x1 = sx;
    m.y2 = sy;
    m.z3 = sz;
    return m;
}

Mat4F mat4_rotate_x(float radians) {
    float c = cosf(radians), s = sinf(radians);
    return (Mat4F){
        1, 0, 0, 0,
        0, c,-s, 0,
        0, s, c, 0,
        0, 0, 0, 1
    };
}

Mat4F mat4_rotate_y(float radians) {
    float c = cosf(radians), s = sinf(radians);
    return (Mat4F){
         c, 0, s, 0,
         0, 1, 0, 0,
        -s, 0, c, 0,
         0, 0, 0, 1
    };
}

Mat4F mat4_rotate_z(float radians) {
    float c = cosf(radians), s = sinf(radians);
    return (Mat4F){
        c,-s, 0, 0,
        s, c, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
}

Mat4F mat4_mul(Mat4F a, Mat4F b) {
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

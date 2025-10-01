#include "math.h"
#include <math.h>


Mat4F mat4_identity() {
    Mat4F m = {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    };
    return m;
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

Mat4F mat4_rotate_z(float radians) {
    Mat4F m = mat4_identity();
    float c = cosf(radians);
    float s = sinf(radians);
    m.x1 = c;  m.x2 = -s;
    m.y1 = s;  m.y2 = c;
    return m;
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

void mat4_apply(const Mat4F* m, float x, float y, float* outX, float* outY) {
    float tx = m->x1*x + m->x2*y + m->x4;
    float ty = m->y1*x + m->y2*y + m->y4;
    *outX = tx;
    *outY = ty;
}


// Simple 2D inverse (assumes affine, no shear in z)
Mat4F mat4_inverse_affine(Mat4F m) {
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

Mat4F mat4_from_rows(const float row1[4], const float row2[4], const float row3[4], const float row4[4]) {
    Mat4F m;
    m.x1 = row1[0]; m.x2 = row1[1]; m.x3 = row1[2]; m.x4 = row1[3];
    m.y1 = row2[0]; m.y2 = row2[1]; m.y3 = row2[2]; m.y4 = row2[3];
    m.z1 = row3[0]; m.z2 = row3[1]; m.z3 = row3[2]; m.z4 = row3[3];
    m.w1 = row4[0]; m.w2 = row4[1]; m.w3 = row4[2]; m.w4 = row4[3];
    return m;
}


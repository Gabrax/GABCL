#pragma once
#include <math.h>

// ---------- Matrix ----------
typedef struct {
    float x1,y1,z1,w1;
    float x2,y2,z2,w2;
    float x3,y3,z3,w3;
    float x4,y4,z4,w4;
} Mat4F;

static inline Mat4F mat4_identity() {
    return (Mat4F){
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    };
}

static inline Mat4F mat4_translate(float tx, float ty, float tz) {
    Mat4F m = mat4_identity();
    m.x4 = tx; m.y4 = ty; m.z4 = tz;
    return m;
}

static inline Mat4F mat4_scale(float sx, float sy, float sz) {
    Mat4F m = mat4_identity();
    m.x1 = sx; m.y2 = sy; m.z3 = sz;
    return m;
}

static inline Mat4F mat4_rotateZ(float radians) {
    Mat4F m = mat4_identity();
    float c = cosf(radians), s = sinf(radians);
    m.x1 = c; m.x2 = -s;
    m.y1 = s; m.y2 = c;
    return m;
}

static inline Mat4F mat4_mul(Mat4F a, Mat4F b) {
    Mat4F r;
    float* ap = (float*)&a;
    float* bp = (float*)&b;
    float* rp = (float*)&r;
    for (int row=0; row<4; ++row) {
        for (int col=0; col<4; ++col) {
            rp[row*4+col] =
                ap[row*4+0]*bp[0*4+col] +
                ap[row*4+1]*bp[1*4+col] +
                ap[row*4+2]*bp[2*4+col] +
                ap[row*4+3]*bp[3*4+col];
        }
    }
    return r;
}

static inline Mat4F mat4_inverse_affine(Mat4F m) {
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

// ---------- Shapes ----------
#define SHAPE_SQUARE   0
#define SHAPE_CIRCLE   1
#define SHAPE_TRIANGLE 2

typedef struct {
    int type;
    Mat4F invModel;     // inverse transform
    unsigned int color; // ARGB
} Shape;

// Helpers for host-side transforms
static inline void shape_translate(Shape* shape, float dx, float dy) {
    Mat4F T = mat4_translate(dx,dy,0);
    Mat4F M = mat4_inverse_affine(shape->invModel);
    M = mat4_mul(T,M);
    shape->invModel = mat4_inverse_affine(M);
}

static inline void shape_rotate(Shape* shape, float rad) {
    Mat4F R = mat4_rotateZ(rad);
    Mat4F M = mat4_inverse_affine(shape->invModel);
    M = mat4_mul(R,M);
    shape->invModel = mat4_inverse_affine(M);
}

static inline void shape_scale(Shape* shape, float s) {
    Mat4F S = mat4_scale(s,s,1);
    Mat4F M = mat4_inverse_affine(shape->invModel);
    M = mat4_mul(S,M);
    shape->invModel = mat4_inverse_affine(M);
}

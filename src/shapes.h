#pragma once
#include "algebra.h"
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#define NUM_SHAPES 3
#define SHAPE_SQUARE 0
#define SHAPE_CIRCLE 1
#define SHAPE_TRIANGLE 2

typedef struct {
    int type;
    Mat4F transform;       
    unsigned int color;
} Shape;

typedef struct {
  Vec3F point[3];
} Triangle;

static inline Triangle make_triangle(Vec3F a, Vec3F b, Vec3F c) {
    Triangle t;
    t.point[0] = a;
    t.point[1] = b;
    t.point[2] = c;
    return t;
}

typedef struct {
  Triangle* triangles;
} Mesh;

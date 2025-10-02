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
    Mat4F mvp;       
    unsigned int color;
} Shape;

void shape_update(Shape* shape);

typedef struct {
  Vec3F point[3];
} Triangle;

static inline Triangle make_triangle(Vec3F p1, Vec3F p2, Vec3F p3) {
    Triangle t;
    t.point[0] = p1;
    t.point[1] = p2;
    t.point[2] = p3;
    return t;
}

typedef struct {
  Triangle* triangles;
} Mesh;

inline void mesh_free(Mesh* mesh)
{
  arrfree(mesh->triangles); 
  mesh->triangles = NULL;   
}

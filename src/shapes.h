#pragma once
#include "algebra.h"
#include "camera.h"
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
    Vec4F vertex[3];     // model space vertices
    Vec4F v_clip[3];    // clip space vertices
    Mat4F transform;    // model transform
    Mat4F mvp;          // Model-View-Projection matrix
    unsigned int color;
} Triangle;

static inline Triangle make_triangle(Vec3F p1, Vec3F p2, Vec3F p3)
{ 
  Triangle t;
  t.vertex[0] = vec3_to_vec4(p1);
  t.vertex[1] = vec3_to_vec4(p2);
  t.vertex[2] = vec3_to_vec4(p3);
  t.transform = mat4_identity();
  t.color = 0xFFFF0000u;
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

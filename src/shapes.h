#pragma once
#include "algebra.h"
#include "camera.h"
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include <stdio.h>

typedef struct {
    Vec4F vertex[3];     // model space vertices
    unsigned int color;
} Triangle;

static inline Triangle make_triangle(Vec3F p1, Vec3F p2, Vec3F p3)
{
  Triangle t;
  t.vertex[0] = vec3_to_vec4(p1);
  t.vertex[1] = vec3_to_vec4(p2);
  t.vertex[2] = vec3_to_vec4(p3);
  return t;
}

typedef struct {
    Triangle* triangles;
    int numTriangles;
} Mesh;

Mesh make_mesh_from_vertices(const Vec3F* verts, int count, unsigned int color)
{
  Mesh mesh = {0}; // start with empty triangle list

  for (int i = 0; i + 2 < count; i += 3) {
      Triangle t = make_triangle(verts[i], verts[i+1], verts[i+2]);
      t.color = color;
      arrpush(mesh.triangles, t);
  }

  mesh.numTriangles = arrlen(mesh.triangles);

  return mesh;
}

void print_mesh(const Mesh* mesh)
{
    for(int i = 0; i < mesh->numTriangles; i++)
    {
        Triangle* t = &mesh->triangles[i];
        printf("Triangle %d:\n", i);
        for(int v = 0; v < 3; v++)
        {
            printf("  Vertex %d: (%f, %f, %f)\n", v,
                   t->vertex[v].x,
                   t->vertex[v].y,
                   t->vertex[v].z);
        }
        printf("  Color: 0x%06X\n", t->color & 0xFFFFFF); // print RGB
    }
}

inline void mesh_free(Mesh* mesh)
{
  arrfree(mesh->triangles); 
  mesh->triangles = NULL;   
}

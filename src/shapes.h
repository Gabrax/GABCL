#pragma once

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include <stdio.h>
#include "gab_math.h"
#include "raylib.h"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stdlib.h>
#include <time.h>

typedef struct {
    f3 vertex[3]; 
    f2 uv[3];
    f3 normal[3];
    Color color;
} Triangle;

typedef struct {
    Triangle* triangles;   
    size_t numTriangles;
    size_t numVertices;
} CustomModel;

inline void custom_model_load(CustomModel* model, const char* filename, Color color)
{
    static int seeded = 0;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = 1;
    }

    const struct aiScene* scene = aiImportFile(
        filename,
        aiProcess_Triangulate |          // convert all faces to triangles
        aiProcess_JoinIdenticalVertices |// merge shared vertices
        aiProcess_GenSmoothNormals |     // generate smooth vertex normals
        aiProcess_ImproveCacheLocality | // better vertex cache usage
        aiProcess_OptimizeMeshes         // reduce draw calls
    );

    if (!scene || scene->mNumMeshes == 0) {
        fprintf(stderr, "Failed to load model: %s\n", filename);
        aiReleaseImport(scene);
        return;
    }

    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        const struct aiMesh* mesh = scene->mMeshes[m];

        for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
            const struct aiFace* face = &mesh->mFaces[f];
            if (face->mNumIndices != 3) continue; // skip non-triangles

            Triangle tri = {0};
            for (int i = 0; i < 3; i++) {
                unsigned int idx = face->mIndices[i];
                if (mesh->mVertices) {
                    tri.vertex[i].x = mesh->mVertices[idx].x;
                    tri.vertex[i].y = mesh->mVertices[idx].y;
                    tri.vertex[i].z = mesh->mVertices[idx].z;
                }
                if (mesh->mNormals) {
                    tri.normal[i].x = mesh->mNormals[idx].x;
                    tri.normal[i].y = mesh->mNormals[idx].y;
                    tri.normal[i].z = mesh->mNormals[idx].z;
                }
                if (mesh->mTextureCoords[0]) {
                    tri.uv[i].x = mesh->mTextureCoords[0][idx].x;
                    tri.uv[i].y = mesh->mTextureCoords[0][idx].y;
                }
            }
            Color c;
            c.r = rand() % 256; // 0-255
            c.g = rand() % 256;
            c.b = rand() % 256;
            c.a = 255;           // fully opaque
            tri.color = c;
            arrpush(model->triangles, tri);
        }
    }

    model->numTriangles = arrlen(model->triangles);
    model->numVertices = model->numTriangles * 3;
    aiReleaseImport(scene);
}

inline void custom_model_print_data(const CustomModel* model)
{
  for (int i = 0; i < model->numTriangles; i++) {
      const Triangle* t = &model->triangles[i];
      printf("Triangle %d:\n", i);

      for (int v = 0; v < 3; v++) {
          printf("  Vertex %d:\n", v);
          printf("    Position: (%f, %f, %f)\n",
                 t->vertex[v].x,
                 t->vertex[v].y,
                 t->vertex[v].z);
          printf("    UV:        (%f, %f)\n",
                 t->uv[v].x,
                 t->uv[v].y);
          printf("    Normal:    (%f, %f, %f)\n",
                 t->normal[v].x,
                 t->normal[v].y,
                 t->normal[v].z);
      }

      printf("  Color: (r=%d g=%d b=%d a=%d)\n\n",
             t->color.r, t->color.g, t->color.b, t->color.a);
  }
}

// free memory
inline void custom_model_free(CustomModel* model)
{
    arrfree(model->triangles);
    model->triangles = NULL;
    model->numTriangles = 0;
}

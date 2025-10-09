#pragma once

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include <stdio.h>
#include "algebra.h"
#include "raylib.h"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

typedef struct {
    Vec3 vertex[3]; 
    Vec2 uv[3];
    Vec3 normal[3];
    Color color;
} Triangle;

static inline Triangle MakeTriangle(Vec3 p1, Vec3 p2, Vec3 p3, Color color)
{
  Triangle t;
  t.vertex[0] = p1;
  t.vertex[1] = p2;
  t.vertex[2] = p3;
  t.color = color;
  return t;
}

typedef struct {
    Triangle* triangles;   
    size_t numTriangles;
} CustomModel;

CustomModel MakeMeshFromVertices(const Vec3* verts, int count, Color color)
{
    CustomModel model = {0};

    for (int i = 0; i + 2 < count; i += 3) {
        Triangle t = MakeTriangle(verts[i], verts[i+1], verts[i+2], color);
        arrpush(model.triangles, t);
    }

    model.numTriangles = arrlen(model.triangles);
    return model;
}

CustomModel LoadModel_Assimp(const char* filename, Color color)
{
    CustomModel model = {0};

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
        return model;
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
            tri.color = color;
            arrpush(model.triangles, tri);
        }
    }

    model.numTriangles = arrlen(model.triangles);
    aiReleaseImport(scene);

    return model;
}

inline CustomModel LoadfromOBJ(const char* filename, Color color)
{
    CustomModel mesh = {0};

    FILE* file = fopen(filename, "r");
    if (!file) {
        TraceLog(LOG_ERROR, "Failed to open OBJ file: %s", filename);
        return mesh;
    }

    Vec3* tempVerts   = NULL;
    Vec2* tempUVs     = NULL;
    Vec3* tempNormals = NULL;

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "v ", 2) == 0) {
            Vec3 v;
            if (sscanf(line + 2, "%f %f %f", &v.x, &v.y, &v.z) == 3)
                arrpush(tempVerts, v);
        } 
        else if (strncmp(line, "vt ", 3) == 0) {
            Vec2 uv;
            if (sscanf(line + 3, "%f %f", &uv.x, &uv.y) == 2)
                arrpush(tempUVs, uv);
        } 
        else if (strncmp(line, "vn ", 3) == 0) {
            Vec3 n;
            if (sscanf(line + 3, "%f %f %f", &n.x, &n.y, &n.z) == 3)
                arrpush(tempNormals, n);
        } 
        else if (strncmp(line, "f ", 2) == 0) {
            char* token = strtok(line + 2, " \t\n");
            int vIdx[4], uvIdx[4], nIdx[4];
            int idxCount = 0;

            while (token && idxCount < 4) {
                int vi = 0, ti = 0, ni = 0;
                if (strstr(token, "//"))
                    sscanf(token, "%d//%d", &vi, &ni);
                else if (sscanf(token, "%d/%d/%d", &vi, &ti, &ni) == 3);
                else if (sscanf(token, "%d/%d", &vi, &ti) == 2);
                else
                    sscanf(token, "%d", &vi);

                if (vi < 0) vi = arrlen(tempVerts) + vi + 1;
                if (ti < 0) ti = arrlen(tempUVs) + ti + 1;
                if (ni < 0) ni = arrlen(tempNormals) + ni + 1;

                vIdx[idxCount]  = vi ? vi - 1 : -1;
                uvIdx[idxCount] = ti ? ti - 1 : -1;
                nIdx[idxCount]  = ni ? ni - 1 : -1;

                idxCount++;
                token = strtok(NULL, " \t\n");
            }

            int triOrder[2][3] = { {0,1,2}, {0,2,3} };
            int triCount = (idxCount == 4 ? 2 : 1);

            for (int t = 0; t < triCount; t++) {
                Triangle tri = {0};
                for (int j = 0; j < 3; j++) {
                    int idx = triOrder[t][j];
                    if (vIdx[idx]  >= 0) tri.vertex[j] = tempVerts[vIdx[idx]];
                    if (uvIdx[idx] >= 0) tri.uv[j]     = tempUVs[uvIdx[idx]];
                    if (nIdx[idx]  >= 0) tri.normal[j] = tempNormals[nIdx[idx]];
                }
                tri.color = color;

                Vec3 a = tri.vertex[0];
                Vec3 b = tri.vertex[1];
                Vec3 c = tri.vertex[2];

                Vec3 ab = Vec3Sub(b, a);
                Vec3 ac = Vec3Sub(c, a);
                Vec3 n  = Vec3Cross(ab, ac);

                if (n.z > 0.0f) { // flip if clockwise (OpenGL convention)
                    Vec3 tmpv = tri.vertex[1];  tri.vertex[1] = tri.vertex[2];  tri.vertex[2] = tmpv;
                    Vec2 tmpuv = tri.uv[1];     tri.uv[1]    = tri.uv[2];      tri.uv[2]    = tmpuv;
                    Vec3 tmpn  = tri.normal[1]; tri.normal[1]= tri.normal[2];   tri.normal[2]= tmpn;
                }

                arrpush(mesh.triangles, tri);
            }
        }
    }

    fclose(file);

    mesh.numTriangles = arrlen(mesh.triangles);
    arrfree(tempVerts);
    arrfree(tempUVs);
    arrfree(tempNormals);

    return mesh;
}

void PrintMesh(const CustomModel* model)
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
inline void MeshFree(CustomModel* model)
{
    arrfree(model->triangles);
    model->triangles = NULL;
    model->numTriangles = 0;
}

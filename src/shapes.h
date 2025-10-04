#pragma once
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include <stdio.h>
#include "raylib.h"
#include "raymath.h"

typedef struct {
    Vector4 vertex[3]; // model-space vertices
    Vector2 uv[3];
    Vector3 normal[3];
    Color color;
    Matrix transform; // model/world transform
} Triangle;

static inline Vector4 Vec3ToVec4(Vector3 v) {
    return (Vector4){ v.x, v.y, v.z, 1.0f };
}

static inline Vector3 Vec4ToVec3(Vector4 v) {
    return (Vector3){ v.x, v.y, v.z };
}

static inline Triangle MakeTriangle(Vector3 p1, Vector3 p2, Vector3 p3, Color color)
{
  Triangle t;
  t.vertex[0] = Vec3ToVec4(p1);
  t.vertex[1] = Vec3ToVec4(p2);
  t.vertex[2] = Vec3ToVec4(p3);
  t.color = color;
  t.transform = MatrixIdentity();
  return t;
}

typedef struct {
    Triangle* triangles;   // stb_ds stretchy buffer
    int numTriangles;
} CustomModel;

static inline Matrix UpdateModelTransform(Matrix* transform, Vector3 position, Vector3 rotation)
{
  Matrix mat = *transform;

  mat = MatrixMultiply(MatrixRotateY(rotation.y * DEG2RAD), mat);
  mat = MatrixMultiply(MatrixRotateX(rotation.x * DEG2RAD), mat);
  mat = MatrixMultiply(MatrixRotateZ(rotation.z * DEG2RAD), mat);

  mat = MatrixMultiply(MatrixTranslate(position.x, position.y, position.z), mat);

  return mat;
}

CustomModel MakeMeshFromVertices(const Vector3* verts, int count, Color color)
{
    CustomModel model = {0};

    for (int i = 0; i + 2 < count; i += 3) {
        Triangle t = MakeTriangle(verts[i], verts[i+1], verts[i+2], color);
        arrpush(model.triangles, t);
    }

    model.numTriangles = arrlen(model.triangles);
    return model;
}

inline CustomModel make_mesh_from_OBJ(const char* filename, Color color)
{
    CustomModel mesh = {0};
    Vector3* tempVerts   = NULL;
    Vector2* tempUVs     = NULL;
    Vector3* tempNormals = NULL;

    FILE* file = fopen(filename, "r");
    if (!file) {
        TraceLog(LOG_ERROR, "Failed to open OBJ file: %s", filename);
        return mesh;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "v ", 2) == 0) {
            Vector3 v;
            if (sscanf(line + 2, "%f %f %f", &v.x, &v.y, &v.z) == 3) {
                arrpush(tempVerts, v);
            }
        } 
        else if (strncmp(line, "vt ", 3) == 0) {
            Vector2 uv;
            if (sscanf(line + 3, "%f %f", &uv.x, &uv.y) == 2) {
                arrpush(tempUVs, uv);
            }
        } 
        else if (strncmp(line, "vn ", 3) == 0) {
            Vector3 n;
            if (sscanf(line + 3, "%f %f %f", &n.x, &n.y, &n.z) == 3) {
                arrpush(tempNormals, n);
            }
        } 
        else if (strncmp(line, "f ", 2) == 0) {
            char* token = strtok(line + 2, " \t\n");
            int vIdx[4], uvIdx[4], nIdx[4];
            int idxCount = 0;

            while (token && idxCount < 4) {
                int vi = 0, ti = 0, ni = 0;

                if (strstr(token, "//")) {
                    sscanf(token, "%d//%d", &vi, &ni); // v//vn
                } else if (sscanf(token, "%d/%d/%d", &vi, &ti, &ni) == 3) {
                    // v/vt/vn
                } else if (sscanf(token, "%d/%d", &vi, &ti) == 2) {
                    // v/vt
                } else {
                    sscanf(token, "%d", &vi); // v
                }

                vIdx[idxCount]  = vi ? vi - 1 : -1;
                uvIdx[idxCount] = ti ? ti - 1 : -1;
                nIdx[idxCount]  = ni ? ni - 1 : -1;

                idxCount++;
                token = strtok(NULL, " \t\n");
            }

            // Triangulate (handle quads as 2 triangles)
            int triOrder[2][3] = { {0,1,2}, {0,2,3} };
            int triCount = (idxCount == 4 ? 2 : 1);

            for (int t = 0; t < triCount; t++) {
                Triangle tri = {0};
                for (int j = 0; j < 3; j++) {
                    int idx = triOrder[t][j];
                    if (vIdx[idx]  >= 0) tri.vertex[j] = Vec3ToVec4(tempVerts[vIdx[idx]]);
                    if (uvIdx[idx] >= 0) tri.uv[j]     = tempUVs[uvIdx[idx]];
                    if (nIdx[idx]  >= 0) tri.normal[j] = tempNormals[nIdx[idx]];
                }
                tri.color = color;

                // --- Fix winding order (force CCW) ---
                Vector3 a = Vec4ToVec3(tri.vertex[0]);
                Vector3 b = Vec4ToVec3(tri.vertex[1]);
                Vector3 c = Vec4ToVec3(tri.vertex[2]);

                
                Vector3 ab = Vector3Subtract(b, a);
                Vector3 ac = Vector3Subtract(c, a);
                Vector3 n  = Vector3CrossProduct(ab, ac);

                if (n.z > 0.0f) { // if wrong, swap
                    Vector4 tmpv = tri.vertex[1];  tri.vertex[1] = tri.vertex[2];  tri.vertex[2] = tmpv;
                    Vector2 tmpuv = tri.uv[1];     tri.uv[1]    = tri.uv[2];      tri.uv[2]    = tmpuv;
                    Vector3 tmpn  = tri.normal[1]; tri.normal[1]= tri.normal[2];   tri.normal[2]= tmpn;
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
            printf("  Vertex %d: (%f, %f, %f)\n", v,
                   t->vertex[v].x,
                   t->vertex[v].y,
                   t->vertex[v].z);
        }
        printf("  Color: (r=%d g=%d b=%d a=%d)\n",
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

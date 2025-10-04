typedef struct { uchar r, g, b, a; } Pixel;
typedef struct { float x, y; } Vec2F;
typedef struct { float x, y, z; } Vec3F;
typedef struct { float x, y, z, w; } Vec4F;

typedef struct Matrix {
    float m0, m4, m8, m12;  // Matrix first row (4 components)
    float m1, m5, m9, m13;  // Matrix second row (4 components)
    float m2, m6, m10, m14; // Matrix third row (4 components)
    float m3, m7, m11, m15; // Matrix fourth row (4 components)
} Matrix;

typedef struct {
    Vec4F vertex[3];     // model space vertices
    Vec2F uv[3];         // texture coords (optional)
    Vec3F normal[3];     // normals
    Pixel color;
    Matrix tranform;
} Triangle;

__kernel void vertex_kernel(__global Triangle* tris,__global float3* projVerts, int numTriangles,int width,int height,__global Matrix* mvp)
{
  int i = get_global_id(0);
  if (i >= numTriangles*3) return;

  int triIdx = i / 3;
  int vertIdx = i % 3;

  __global Triangle* tri = &tris[triIdx];
    float4 vert = (float4)(tri->vertex[vertIdx].x,
                           tri->vertex[vertIdx].y,
                           tri->vertex[vertIdx].z,
                           1.0f);

    float x = vert.x*mvp->m0 + vert.y*mvp->m4 + vert.z*mvp->m8  + vert.w*mvp->m12;
    float y = vert.x*mvp->m1 + vert.y*mvp->m5 + vert.z*mvp->m9  + vert.w*mvp->m13;
    float z = vert.x*mvp->m2 + vert.y*mvp->m6 + vert.z*mvp->m10 + vert.w*mvp->m14;
    float w = vert.x*mvp->m3 + vert.y*mvp->m7 + vert.z*mvp->m11 + vert.w*mvp->m15;

    // Guard against w = 0
    if (fabs(w) < 1e-6f) {
        projVerts[i] = (float3)(-1e9f, -1e9f, 1e9f);
        return;
    }

    float invW = 1.0f / w;

    // Perspective divide to NDC
    float ndcX = x * invW;
    float ndcY = y * invW;
    float ndcZ = z * invW;

    // Map to screen coordinates (flip Y)
    float sx = (ndcX + 1.0f) * 0.5f * (float)width;
    float sy = (1.0f - (ndcY + 1.0f) * 0.5f) * (float)height;

    // Clamp Z to [0,1] for depth buffer
    ndcZ = ndcZ * 0.5f + 0.5f;
    ndcZ = fmin(fmax(ndcZ, 0.0f), 1.0f);

    projVerts[i] = (float3)(sx, sy, ndcZ);
}

__kernel void fragment_kernel(
    __global Pixel* pixels,
    __global float* depth,
    __global Triangle* tris,
    __global float3* projVerts,
    int numTriangles,
    int width,
    int height)
{
    int x = get_global_id(0);
    int y = get_global_id(1);
    if (x >= width || y >= height) return;

    float px = (float)x;
    float py = (float)y;

    float mindepth = 1e10f;
    Pixel color = (Pixel){0, 0, 0, 255}; // background

    for (int tri = 0; tri < numTriangles; tri++)
    {
        __global Triangle* t = &tris[tri];

        float2 v0 = (float2)(projVerts[tri*3 + 0].x, projVerts[tri*3 + 0].y);
        float2 v1 = (float2)(projVerts[tri*3 + 1].x, projVerts[tri*3 + 1].y);
        float2 v2 = (float2)(projVerts[tri*3 + 2].x, projVerts[tri*3 + 2].y);

        float den = (v1.x - v0.x)*(v2.y - v0.y) - (v2.x - v0.x)*(v1.y - v0.y);
        if (fabs(den) < 1e-6f) continue;

        float a = ((v2.y - v0.y)*(px - v0.x) - (v2.x - v0.x)*(py - v0.y)) / den;
        float b = (-(v1.y - v0.y)*(px - v0.x) + (v1.x - v0.x)*(py - v0.y)) / den;
        float g = 1.0f - a - b;

        if ((a >= 0 && b >= 0 && g >= 0) || (a <= 0 && b <= 0 && g <= 0))
        {
            float z = a*projVerts[tri*3 + 0].z +
                      b*projVerts[tri*3 + 1].z +
                      g*projVerts[tri*3 + 2].z;

            if (z < mindepth)
            {
                mindepth = z;
                color = t->color;   // <-- directly use the struct Pixel
            }
        }
    }

    pixels[y*width + x] = color;
    depth[y*width + x] = mindepth;
}

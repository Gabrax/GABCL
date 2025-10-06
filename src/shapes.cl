typedef struct { uchar r, g, b, a; } Pixel;
typedef struct { float x, y; } Vec2;
typedef struct { float x, y, z; } Vec3;

typedef struct Mat4 {
    float x0, x1, x2, x3;
    float y0, y1, y2, y3;
    float z0, z1, z2, z3;
    float w0, w1, w2, w3;
} Mat4;

typedef struct {
    Vec3 vertex[3];
    Vec2 uv[3];
    Vec3 normal[3];
    Pixel color;
} Triangle;

__kernel void vertex_kernel(__global Triangle* tris,__global float3* projVerts, int numTriangles,int width,int height,__global Mat4* mvp)
{
  int i = get_global_id(0);
  if (i >= numTriangles*3) return;

  int triIdx = i / 3, vertIdx = i % 3;

  __global Triangle* tri = &tris[triIdx];
  float3 vert = (float3)(tri->vertex[vertIdx].x,tri->vertex[vertIdx].y,tri->vertex[vertIdx].z);

  // --- PROJECTION MULTIPLICATION ---
  float x = vert.x * mvp->x0 + vert.y * mvp->x1 + vert.z * mvp->x2 + mvp->x3;
  float y = vert.x * mvp->y0 + vert.y * mvp->y1 + vert.z * mvp->y2 + mvp->y3;
  float z = vert.x * mvp->z0 + vert.y * mvp->z1 + vert.z * mvp->z2 + mvp->z3;
  float w = vert.x * mvp->w0 + vert.y * mvp->w1 + vert.z * mvp->w2 + mvp->w3;

  // --- Perspective divide ---
  if (w != 0.0f) { x /= w; y /= w; z /= w;}

  // --- NDC → screen space ---
  float sx = (x * 0.5f + 0.5f) * (float)width;
  float sy = (y * 0.5f + 0.5f) * (float)height;
  float sz = z * 0.5f + 0.5f;

  projVerts[i] = (float3)(sx, sy, sz);
}

/*__kernel void fragment_kernel(*/
/*    __global Pixel* pixels,*/
/*    __global float* depth,*/
/*    __global Triangle* tris,*/
/*    __global float3* projVerts,*/
/*    int numTriangles,*/
/*    int width,*/
/*    int height)*/
/*{*/
/*    int x = get_global_id(0);*/
/*    int y = get_global_id(1);*/
/*    if (x >= width || y >= height) return;*/
/**/
/*    float px = (float)x;*/
/*    float py = (float)y;*/
/**/
/*    float mindepth = 1e10f;*/
/*    Pixel color = (Pixel){0, 0, 0, 255}; // background*/
/**/
/*    for (int tri = 0; tri < numTriangles; tri++)*/
/*    {*/
/*        __global Triangle* t = &tris[tri];*/
/**/
/*        float2 v0 = (float2)(projVerts[tri*3 + 0].x, projVerts[tri*3 + 0].y);*/
/*        float2 v1 = (float2)(projVerts[tri*3 + 1].x, projVerts[tri*3 + 1].y);*/
/*        float2 v2 = (float2)(projVerts[tri*3 + 2].x, projVerts[tri*3 + 2].y);*/
/**/
/*        float den = (v1.x - v0.x)*(v2.y - v0.y) - (v2.x - v0.x)*(v1.y - v0.y);*/
/*        if (fabs(den) < 1e-6f) continue;*/
/**/
/*        float a = ((v2.y - v0.y)*(px - v0.x) - (v2.x - v0.x)*(py - v0.y)) / den;*/
/*        float b = (-(v1.y - v0.y)*(px - v0.x) + (v1.x - v0.x)*(py - v0.y)) / den;*/
/*        float g = 1.0f - a - b;*/
/**/
/*        if ((a >= 0 && b >= 0 && g >= 0) || (a <= 0 && b <= 0 && g <= 0))*/
/*        {*/
/*            float z = a*projVerts[tri*3 + 0].z +*/
/*                      b*projVerts[tri*3 + 1].z +*/
/*                      g*projVerts[tri*3 + 2].z;*/
/**/
/*            if (z < mindepth)*/
/*            {*/
/*                mindepth = z;*/
/*                color = t->color;   */
/*            }*/
/*        }*/
/*    }*/
/**/
/*    pixels[y*width + x] = color;*/
/*    depth[y*width + x] = mindepth;*/
/*}*/


// WIREFRAME
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

  Pixel color = (Pixel){0, 0, 0, 255}; // background
  float mindepth = 1e10f;
  float edgeThickness = 1.0f; // pixels wide

  for (int tri = 0; tri < numTriangles; tri++)
  {
      __global Triangle* t = &tris[tri];

      float3 v0 = projVerts[tri*3 + 0];
      float3 v1 = projVerts[tri*3 + 1];
      float3 v2 = projVerts[tri*3 + 2];

      // 2D projected positions
      float2 p0 = (float2)(v0.x, v0.y);
      float2 p1 = (float2)(v1.x, v1.y);
      float2 p2 = (float2)(v2.x, v2.y);

      // Compute barycentric coordinates
      float den = (p1.x - p0.x)*(p2.y - p0.y) - (p2.x - p0.x)*(p1.y - p0.y);
      if (fabs(den) < 1e-6f) continue;

      float a = ((p2.y - p0.y)*(px - p0.x) - (p2.x - p0.x)*(py - p0.y)) / den;
      float b = (-(p1.y - p0.y)*(px - p0.x) + (p1.x - p0.x)*(py - p0.y)) / den;
      float g = 1.0f - a - b;

      // Check if inside triangle bounds (with small margin)
      if (a >= -0.01f && b >= -0.01f && g >= -0.01f)
      {
          // Distance from edges in barycentric space
          float da = fabs(a);
          float db = fabs(b);
          float dg = fabs(g);

          // If near any edge (within thickness)
          if (a < edgeThickness/100.0f || b < edgeThickness/100.0f || g < edgeThickness/100.0f)
          {
              float z = a*v0.z + b*v1.z + g*v2.z;
              if (z < mindepth)
              {
                  mindepth = z;
                  color = t->color; // triangle color for wire
              }
          }
      }
  }

  pixels[y*width + x] = color;
  depth[y*width + x] = mindepth;
}

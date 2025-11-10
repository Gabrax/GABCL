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

__kernel void clear_buffers(
    __global Pixel* pixels,
    __global float* depth,
    int width, int height,
    Pixel color,
    float depthClear
) {
    int x = get_global_id(0);
    int y = get_global_id(1);
    if (x >= width || y >= height) return;
    int idx = y * width + x;
    pixels[idx] = color;
    depth[idx] = depthClear;
}

__kernel void vertex_kernel(
    __global Triangle* tris,
    __global float4* projVerts,
    __global Mat4* projection,
    __global Mat4* view,
    __global Mat4* transform, // model matrix
    int numTriangles,
    int width,
    int height,
    __global float3* cameraPos,__global float3* fragPos)
{
  int i = get_global_id(0);
  if (i >= numTriangles * 3) return;

  int triIdx = i / 3;
  int vertIdx = i % 3;

  __global Triangle* tri = &tris[triIdx];
  float4 vert = (float4)(tri->vertex[vertIdx].x,tri->vertex[vertIdx].y,tri->vertex[vertIdx].z, 1.0f);

  float4 v_model;
  v_model.x = vert.x * transform->x0 + vert.y * transform->x1 + vert.z * transform->x2 + vert.w * transform->x3;
  v_model.y = vert.x * transform->y0 + vert.y * transform->y1 + vert.z * transform->y2 + vert.w * transform->y3;
  v_model.z = vert.x * transform->z0 + vert.y * transform->z1 + vert.z * transform->z2 + vert.w * transform->z3;
  v_model.w = vert.x * transform->w0 + vert.y * transform->w1 + vert.z * transform->w2 + vert.w * transform->w3;

  float3 normal = (float3)(tri->normal[vertIdx].x,tri->normal[vertIdx].y,tri->normal[vertIdx].z);
  float3 normal_world = normalize((float3)(
      normal.x * transform->x0 + normal.y * transform->x1 + normal.z * transform->x2,
      normal.x * transform->y0 + normal.y * transform->y1 + normal.z * transform->y2,
      normal.x * transform->z0 + normal.y * transform->z1 + normal.z * transform->z2
  ));

  float3 viewDir = normalize(*cameraPos - (float3)(v_model.x, v_model.y, v_model.z));
  if (dot(normal_world, viewDir) < 0.0f) {
      projVerts[i] = (float4)(-9999.0f, -9999.0f, -9999.0f, -9999.0f);
      return;
  }

  float4 v_view;
  v_view.x = v_model.x * view->x0 + v_model.y * view->x1 + v_model.z * view->x2 + v_model.w * view->x3;
  v_view.y = v_model.x * view->y0 + v_model.y * view->y1 + v_model.z * view->y2 + v_model.w * view->y3;
  v_view.z = v_model.x * view->z0 + v_model.y * view->z1 + v_model.z * view->z2 + v_model.w * view->z3;
  v_view.w = v_model.x * view->w0 + v_model.y * view->w1 + v_model.z * view->w2 + v_model.w * view->w3;

  float4 v_clip;
  v_clip.x = v_view.x * projection->x0 + v_view.y * projection->x1 + v_view.z * projection->x2 + v_view.w * projection->x3;
  v_clip.y = v_view.x * projection->y0 + v_view.y * projection->y1 + v_view.z * projection->y2 + v_view.w * projection->y3;
  v_clip.z = v_view.x * projection->z0 + v_view.y * projection->z1 + v_view.z * projection->z2 + v_view.w * projection->z3;
  v_clip.w = v_view.x * projection->w0 + v_view.y * projection->w1 + v_view.z * projection->w2 + v_view.w * projection->w3;

  // --- Perspective divide ---
  float ndc_x = v_clip.x / v_clip.w;
  float ndc_y = v_clip.y / v_clip.w;
  float ndc_z = v_clip.z / v_clip.w;

  // --- NDC → screen space ---
  float sx = (ndc_x * 0.5f + 0.5f) * (float)width;
  float sy = (ndc_y * 0.5f + 0.5f) * (float)height;
  float sz = ndc_z * 0.5f + 0.5f;

  fragPos[i] = (float3)(v_view.x, v_view.y, v_view.z);
  projVerts[i] = (float4)(sx, sy, sz, v_clip.w);
}

inline float3 reflect(float3 I, float3 N)
{
    return I - 2.0f * dot(N, I) * N;
}

inline float SignedTriangleArea(float2 a, float2 b, float2 c)
{
    return 0.5f * ((b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x));
}

__kernel void fragment_kernel(
    __global Pixel* pixels,
    __global Triangle* tris,
    __global float4* projVerts,
    int numTriangles,
    int width,
    int height,
    __global float* depthBuffer,
    __global float3* cameraPos,
    __global float3* fragPos)
{
    int x = get_global_id(0);
    int y = get_global_id(1);
    if (x >= width || y >= height) return;

    int idx = y * width + x;
    float2 P = (float2)(x + 0.5f, y + 0.5f); // pixel center

    for (int tri = 0; tri < numTriangles; tri++)
    {
        float4 pv0 = projVerts[tri*3 + 0];
        float4 pv1 = projVerts[tri*3 + 1];
        float4 pv2 = projVerts[tri*3 + 2];

        // Skip culled
        if (pv0.w == -9999.0f || pv1.w == -9999.0f || pv2.w == -9999.0f)
            continue;

        float2 v0 = (float2)(pv0.x, pv0.y);
        float2 v1 = (float2)(pv1.x, pv1.y);
        float2 v2 = (float2)(pv2.x, pv2.y);

        float area = SignedTriangleArea(v0, v1, v2);
        if (area <= 0.0f) continue;

        float a = SignedTriangleArea(P, v1, v2) / area;
        float b = SignedTriangleArea(P, v2, v0) / area;
        float g = SignedTriangleArea(P, v0, v1) / area;

        if (a >= 0 && b >= 0 && g >= 0)
        {
            float z0 = pv0.z / pv0.w;
            float z1 = pv1.z / pv1.w;
            float z2 = pv2.z / pv2.w;
            float depth = a*z0 + b*z1 + g*z2;

            if (depth < depthBuffer[idx])
            {
                __global Triangle* t = &tris[tri];
                float3 baseColor = (float3)(t->color.r / 255.0f, t->color.g / 255.0f, t->color.b / 255.0f);

                pixels[idx] = (Pixel){
                    (uchar)(baseColor.x * 255.0f),
                    (uchar)(baseColor.y * 255.0f),
                    (uchar)(baseColor.z * 255.0f),
                    255
                };
                depthBuffer[idx] = depth;
            }
        }
    }
}
// WIREFRAME
/*__kernel void fragment_kernel(*/
/*    __global Pixel* pixels,*/
/*    __global Triangle* tris,*/
/*    __global float3* projVerts,*/
/*    int numTriangles,*/
/*    int width,*/
/*    int height)*/
/*{*/
/*  int x = get_global_id(0);*/
/*  int y = get_global_id(1);*/
/*  if (x >= width || y >= height) return;*/
/**/
/*  float px = (float)x;*/
/*  float py = (float)y;*/
/**/
/*  float edgeThickness = 0.5f; // pixels wide*/
/**/
/*  for (int tri = 0; tri < numTriangles; tri++)*/
/*  {*/
/*    __global Triangle* t = &tris[tri];*/
/**/
/*    float3 v0 = projVerts[tri*3 + 0];*/
/*    float3 v1 = projVerts[tri*3 + 1];*/
/*    float3 v2 = projVerts[tri*3 + 2];*/
/**/
/*    float3 n0 = normalize((float3)(t->normal[tri*3 + 0].x,t->normal[tri*3 + 0].y,t->normal[tri*3 + 0].z));*/
/*    float3 n1 = normalize((float3)(t->normal[tri*3 + 1].x,t->normal[tri*3 + 1].y,t->normal[tri*3 + 1].z));*/
/*    float3 n2 = normalize((float3)(t->normal[tri*3 + 2].x,t->normal[tri*3 + 2].y,t->normal[tri*3 + 2].z));*/
/**/
/*    // 2D projected positions*/
/*    float2 p0 = (float2)(v0.x, v0.y);*/
/*    float2 p1 = (float2)(v1.x, v1.y);*/
/*    float2 p2 = (float2)(v2.x, v2.y);*/
/**/
/*    // Compute barycentric coordinates*/
/*    float den = (p1.x - p0.x)*(p2.y - p0.y) - (p2.x - p0.x)*(p1.y - p0.y);*/
/*    if (fabs(den) < 1e-6f) continue;*/
/**/
/*    float a = ((p2.y - p0.y)*(px - p0.x) - (p2.x - p0.x)*(py - p0.y)) / den;*/
/*    float b = (-(p1.y - p0.y)*(px - p0.x) + (p1.x - p0.x)*(py - p0.y)) / den;*/
/*    float g = 1.0f - a - b;*/
/**/
/*    // Check if inside triangle bounds (with small margin)*/
/*    if (a >= -0.01f && b >= -0.01f && g >= -0.01f)*/
/*    {*/
/*      // Distance from edges in barycentric space*/
/*      float da = fabs(a);*/
/*      float db = fabs(b);*/
/*      float dg = fabs(g);*/
/**/
/*      // If near any edge (within thickness)*/
/*      if (a < edgeThickness/100.0f || b < edgeThickness/100.0f || g < edgeThickness/100.0f)*/
/*      {*/
/*          pixels[y*width + x] = t->color; // triangle color for wire*/
/*      }*/
/*    }*/
/*  }*/
/*}*/

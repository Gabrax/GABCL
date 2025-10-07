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

__kernel void vertex_kernel(
    __global Triangle* tris,
    __global float3* projVerts,
    __global Mat4* projection,
    __global Mat4* view,
    __global Mat4* transform,
    int numTriangles,
    int width,
    int height)
{
    int triIdx = get_global_id(0);
    if (triIdx >= numTriangles) return;

    __global Triangle* tri = &tris[triIdx];

    // --- Compute triangle normal (using vertex normals) ---
    float3 n0 = (float3)(tri->normal[0].x, tri->normal[0].y, tri->normal[0].z);
    float3 n1 = (float3)(tri->normal[1].x, tri->normal[1].y, tri->normal[1].z);
    float3 n2 = (float3)(tri->normal[2].x, tri->normal[2].y, tri->normal[2].z);

    // Average
    float3 normal = (n0 + n1 + n2) / 3.0f;

    // Transform normal to view space (ignore translation, only rotation)
    float3 n_view;
    n_view.x = normal.x * view->x0 + normal.y * view->x1 + normal.z * view->x2;
    n_view.y = normal.x * view->y0 + normal.y * view->y1 + normal.z * view->y2;
    n_view.z = normal.x * view->z0 + normal.y * view->z1 + normal.z * view->z2;

    // --- Backface culling ---
    float3 viewDir = (float3)(0.0f, 0.0f, 1.0f); // camera looks down -Z
    if (dot(n_view, viewDir) > 0.0f) {
        // Cull triangle: write invalid vertices
        for (int j = 0; j < 3; j++) {
            projVerts[triIdx * 3 + j] = (float3)(-1.0f, -1.0f, -1.0f);
        }
        return;
    }

    // --- Transform all 3 vertices: Model → View → Projection ---
    for (int j = 0; j < 3; j++) {
        float4 vert = (float4)(tri->vertex[j].x, tri->vertex[j].y, tri->vertex[j].z, 1.0f);

        // Model → View
        float4 v_model;
        v_model.x = vert.x * transform->x0 + vert.y * transform->x1 + vert.z * transform->x2 + vert.w * transform->x3;
        v_model.y = vert.x * transform->y0 + vert.y * transform->y1 + vert.z * transform->y2 + vert.w * transform->y3;
        v_model.z = vert.x * transform->z0 + vert.y * transform->z1 + vert.z * transform->z2 + vert.w * transform->z3;
        v_model.w = vert.x * transform->w0 + vert.y * transform->w1 + vert.z * transform->w2 + vert.w * transform->w3;

        float4 v_view;
        v_view.x = v_model.x * view->x0 + v_model.y * view->x1 + v_model.z * view->x2 + v_model.w * view->x3;
        v_view.y = v_model.x * view->y0 + v_model.y * view->y1 + v_model.z * view->y2 + v_model.w * view->y3;
        v_view.z = v_model.x * view->z0 + v_model.y * view->z1 + v_model.z * view->z2 + v_model.w * view->z3;
        v_view.w = v_model.x * view->w0 + v_model.y * view->w1 + v_model.z * view->w2 + v_model.w * view->w3;

        // View → Clip
        float4 v_clip;
        v_clip.x = v_view.x * projection->x0 + v_view.y * projection->x1 + v_view.z * projection->x2 + v_view.w * projection->x3;
        v_clip.y = v_view.x * projection->y0 + v_view.y * projection->y1 + v_view.z * projection->y2 + v_view.w * projection->y3;
        v_clip.z = v_view.x * projection->z0 + v_view.y * projection->z1 + v_view.z * projection->z2 + v_view.w * projection->z3;
        v_clip.w = v_view.x * projection->w0 + v_view.y * projection->w1 + v_view.z * projection->w2 + v_view.w * projection->w3;

        // Perspective divide
        float ndc_x = v_clip.x / v_clip.w;
        float ndc_y = v_clip.y / v_clip.w;
        float ndc_z = v_clip.z / v_clip.w;

        // NDC → screen space
        float sx = (ndc_x * 0.5f + 0.5f) * (float)width;
        float sy = (ndc_y * 0.5f + 0.5f) * (float)height;
        float sz = ndc_z * 0.5f + 0.5f;

        projVerts[triIdx * 3 + j] = (float3)(sx, sy, sz);
    }
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

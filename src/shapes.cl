typedef struct { uchar r, g, b, a; } Pixel;
typedef struct {
    float x, y, z, w;
} Vec4F;

typedef struct {
    Vec4F vertex[3];   // 3 vertices in model space
    unsigned int color;  // ARGB
} Triangle;

__kernel void vertex_kernel(
    __global Triangle* tris,
    __global float3* projVerts, // numTriangles * 3
    int numTriangles,
    float t,
    int width,
    int height)
{
    int i = get_global_id(0); // i = vertex index across all triangles
    if (i >= numTriangles * 3) return;

    int triIdx = i / 3;
    int vertIdx = i % 3;

    __global Triangle* tri = &tris[triIdx];
    float3 vert = (float3)(tri->vertex[vertIdx].x,
                           tri->vertex[vertIdx].y,
                           tri->vertex[vertIdx].z);

    // Rotation
    float s = sin(t), c = cos(t);
    float sX = sin(t*0.7f), cX = cos(t*0.7f);
    float y0 = vert.y * cX - vert.z * sX;
    float z0 = vert.y * sX + vert.z * cX;
    float x0 = vert.x;
    float x1 = x0 * c - z0 * s;
    float z1 = x0 * s + z0 * c;

    // Perspective projection
    float f = 1.0f / (z1 + 2.0f);
    projVerts[i] = (float3)((x1*f + 0.5f) * (float)width,(y0*f + 0.5f) * (float)height,z1);
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
    Pixel color = {0,0,0,255};

    for (int tri = 0; tri < numTriangles; tri++)
    {
        __global Triangle* t = &tris[tri];
        unsigned int c = t->color;

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
            float z = a*projVerts[tri*3 + 0].z + b*projVerts[tri*3 + 1].z + g*projVerts[tri*3 + 2].z;
            if (z < mindepth)
            {
                mindepth = z;
                color.r = (c >> 16) & 0xFF;
                color.g = (c >> 8) & 0xFF;
                color.b = c & 0xFF;
                color.a = 255;
            }
        }
    }

    pixels[y*width + x] = color;
    depth[y*width + x] = mindepth;
}

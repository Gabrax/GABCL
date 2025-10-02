typedef struct {
    float x1, y1, z1, w1;
    float x2, y2, z2, w2;
    float x3, y3, z3, w3;
    float x4, y4, z4, w4;
} Mat4F;

typedef struct {
    float x, y, z, w;
} Vec4F;

typedef struct {
    Vec4F vertex[3];     // model space vertices
    Vec4F v_clip[3];    // clip space vertices
    Mat4F transform;    // model transform
    Mat4F mvp;          // Model-View-Projection matrix
    unsigned int color;
} Triangle;

typedef struct {
    int type;
    Mat4F transform;       
    unsigned int color;
} Shape;

Mat4F mat4_mul(Mat4F a, Mat4F b)
{
  Mat4F r;
  r.x1 = a.x1*b.x1 + a.x2*b.y1 + a.x3*b.z1 + a.x4*b.w1;
  r.x2 = a.x1*b.x2 + a.x2*b.y2 + a.x3*b.z2 + a.x4*b.w2;
  r.x3 = a.x1*b.x3 + a.x2*b.y3 + a.x3*b.z3 + a.x4*b.w3;
  r.x4 = a.x1*b.x4 + a.x2*b.y4 + a.x3*b.z4 + a.x4*b.w4;

  r.y1 = a.y1*b.x1 + a.y2*b.y1 + a.y3*b.z1 + a.y4*b.w1;
  r.y2 = a.y1*b.x2 + a.y2*b.y2 + a.y3*b.z2 + a.y4*b.w2;
  r.y3 = a.y1*b.x3 + a.y2*b.y3 + a.y3*b.z3 + a.y4*b.w3;
  r.y4 = a.y1*b.x4 + a.y2*b.y4 + a.y3*b.z4 + a.y4*b.w4;

  r.z1 = a.z1*b.x1 + a.z2*b.y1 + a.z3*b.z1 + a.z4*b.w1;
  r.z2 = a.z1*b.x2 + a.z2*b.y2 + a.z3*b.z2 + a.z4*b.w2;
  r.z3 = a.z1*b.x3 + a.z2*b.y3 + a.z3*b.z3 + a.z4*b.w3;
  r.z4 = a.z1*b.x4 + a.z2*b.y4 + a.z3*b.z4 + a.z4*b.w4;

  r.w1 = a.w1*b.x1 + a.w2*b.y1 + a.w3*b.z1 + a.w4*b.w1;
  r.w2 = a.w1*b.x2 + a.w2*b.y2 + a.w3*b.z2 + a.w4*b.w2;
  r.w3 = a.w1*b.x3 + a.w2*b.y3 + a.w3*b.z3 + a.w4*b.w3;
  r.w4 = a.w1*b.x4 + a.w2*b.y4 + a.w3*b.z4 + a.w4*b.w4;
  return r;
}
Vec4F mat4_mul_vec4(Mat4F mat, Vec4F vec)
{
  Vec4F res;
  res.x = mat.x1 * vec.x + mat.x2 * vec.y + mat.x3 * vec.z + mat.x4 * vec.w;
  res.y = mat.y1 * vec.x + mat.y2 * vec.y + mat.y3 * vec.z + mat.y4 * vec.w;
  res.z = mat.z1 * vec.x + mat.z2 * vec.y + mat.z3 * vec.z + mat.z4 * vec.w;
  res.w = mat.w1 * vec.x + mat.w2 * vec.y + mat.w3 * vec.z + mat.w4 * vec.w;
  return res;
}
inline Mat4F mat4_inverse_affine(const Mat4F m) {
    Mat4F inv;

    // Extract 2x2 rotation/scale
    float a = m.x1, b = m.x2;
    float c = m.y1, d = m.y2;

    float det = a * d - b * c;
    float invDet = 1.0f / det;

    // Inverse rotation/scale
    inv.x1 =  d * invDet; inv.x2 = -b * invDet; inv.x3 = 0; inv.x4 = 0;
    inv.y1 = -c * invDet; inv.y2 =  a * invDet; inv.y3 = 0; inv.y4 = 0;
    inv.z1 =  0;          inv.z2 =  0;          inv.z3 = 1; inv.z4 = 0;

    // Inverse translation = -(R^-1 * t)
    float tx = m.x4;
    float ty = m.y4;
    inv.x4 = -(inv.x1 * tx + inv.x2 * ty);
    inv.y4 = -(inv.y1 * tx + inv.y2 * ty);
    inv.z4 = 0;

    // last row (homogeneous)
    inv.w1 = 0; inv.w2 = 0; inv.w3 = 0; inv.w4 = 1;

    return inv;
}

inline void apply_inv(const Shape sh, float px, float py, float* ox, float* oy) {
    float p[4] = { px, py, 0.0f, 1.0f };
    *ox = sh.transform.x1*p[0] + sh.transform.x2*p[1] + sh.transform.x3*p[2] + sh.transform.x4*p[3];
    *oy = sh.transform.y1*p[0] + sh.transform.y2*p[1] + sh.transform.y3*p[2] + sh.transform.y4*p[3];
}

inline uint draw_square(uint pixel, const Shape sh, int px, int py) {
    float ox, oy; apply_inv(sh, px, py, &ox, &oy);
    if(ox >= -0.5f && ox <= 0.5f && oy >= -0.5f && oy <= 0.5f) pixel = sh.color;
    return pixel;
}

inline uint draw_circle(uint pixel, const Shape sh, int px, int py) {
    float ox, oy; apply_inv(sh, px, py, &ox, &oy);
    if(ox*ox + oy*oy <= 0.25f) pixel = sh.color;
    return pixel;
}

inline uint draw_triangle(uint pixel, const Shape sh, int px, int py) {
    float ox, oy; apply_inv(sh, px, py, &ox, &oy);
    if(oy >= -0.5f && oy <= 0.5f) {
        float t = oy + 0.5f;
        float halfBase = 0.5f * (1.0f - t);
        if(ox >= -halfBase && ox <= halfBase) pixel = sh.color;
    }
    return pixel;
}

__kernel void draw_shapes(__global uint* buf,
                          __global const Shape* shapes,
                          const int numShapes,
                          const int w, const int h) {
    int px = get_global_id(0);
    int py = get_global_id(1);
    if(px >= w || py >= h) return;

    uint pixel = 0;

    for(int i = 0; i < numShapes; i++){
        // Copy shape and precompute inverse ONCE
        Shape sh = shapes[i];
        sh.transform = mat4_inverse_affine(sh.transform);

        // Test pixel against shape in local space
        if(sh.type == 0) pixel = draw_square(pixel, sh, px, py);
        else if(sh.type == 1) pixel = draw_circle(pixel, sh, px, py);
        else if(sh.type == 2) pixel = draw_triangle(pixel, sh, px, py);
    }

    buf[py*w + px] = pixel;
}

__kernel void transform_triangles(__global Triangle* tris,const Mat4F viewProj,const int numTris)
{
    int id = get_global_id(0);
    if (id >= numTris) return;

    // Work on global triangle
    __global Triangle* t_global = &tris[id];

    // Compute MVP in private memory
    Mat4F mvp = mat4_mul(viewProj, t_global->transform);

    // Temporary private array for transformed vertices
    Vec4F v_clip_private[3];

    for (int v = 0; v < 3; v++) {
        // Copy vertex to private memory
        Vec4F vertex_private = t_global->vertex[v];

        // Transform to clip space
        v_clip_private[v] = mat4_mul_vec4(mvp, vertex_private);

        // Perspective divide in private memory
        float w = v_clip_private[v].w;
        v_clip_private[v].x /= w;
        v_clip_private[v].y /= w;
        v_clip_private[v].z /= w;
        v_clip_private[v].w = 1.0f;
    }

    // Write results back to global memory
    t_global->mvp = mvp;
    for (int v = 0; v < 3; v++) t_global->v_clip[v] = v_clip_private[v];
}

inline bool point_in_triangle(float x, float y, Vec4F v0, Vec4F v1, Vec4F v2)
{
  // barycentric method
  float denom = (v1.y - v2.y)*(v0.x - v2.x) + (v2.x - v1.x)*(v0.y - v2.y);
  float a = ((v1.y - v2.y)*(x - v2.x) + (v2.x - v1.x)*(y - v2.y)) / denom;
  float b = ((v2.y - v0.y)*(x - v2.x) + (v0.x - v2.x)*(y - v2.y)) / denom;
  float c = 1.0f - a - b;
  return a >= 0 && b >= 0 && c >= 0;
}

__kernel void rasterize_triangles(__global uint* framebuffer,__global const Triangle* tris,const int numTris,const int width,const int height)
{
  int px = get_global_id(0);
  int py = get_global_id(1);
  if (px >= width || py >= height) return;

  // Convert pixel to NDC coords
  float ndcX = (2.0f * px) / width - 1.0f;
  float ndcY = 1.0f - (2.0f * py) / height; // flip Y

  uint pixel = 0; // background

  // Loop over triangles
  for (int i = 0; i < numTris; i++) {
      const Triangle t = tris[i];

      if (point_in_triangle(ndcX, ndcY, t.v_clip[0], t.v_clip[1], t.v_clip[2])) {
          pixel = t.color;
          /*break; // simple "painter's algorithm"*/
      }
  }

  framebuffer[py * width + px] = pixel;
}

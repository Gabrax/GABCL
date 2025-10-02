typedef struct {
    float x1, y1, z1, w1;
    float x2, y2, z2, w2;
    float x3, y3, z3, w3;
    float x4, y4, z4, w4;
} Mat4F;

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

typedef struct {
    int type;
    Mat4F transform;       
    unsigned int color;
} Shape;

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

typedef struct {
    int type;
    float row1[4];
    float row2[4];
    float row3[4];
    float row4[4];
    uint color;
} Shape;

inline void apply_inv(const Shape sh, float px, float py, float* ox, float* oy) {
    float p[4] = {px, py, 0.0f, 1.0f};
    *ox = sh.row1[0]*p[0] + sh.row1[1]*p[1] + sh.row1[2]*p[2] + sh.row1[3]*p[3];
    *oy = sh.row2[0]*p[0] + sh.row2[1]*p[1] + sh.row2[2]*p[2] + sh.row2[3]*p[3];
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

__kernel void draw_shapes(__global uint* buf, __global const Shape* shapes, const int numShapes, const int w, const int h) {
    int px = get_global_id(0);
    int py = get_global_id(1);
    if(px >= w || py >= h) return;
    uint pixel = 0;
    for(int i = 0; i < numShapes; i++){
        Shape sh = shapes[i];
        if(sh.type == 0) pixel = draw_square(pixel, sh, px, py);
        else if(sh.type == 1) pixel = draw_circle(pixel, sh, px, py);
        else if(sh.type == 2) pixel = draw_triangle(pixel, sh, px, py);
    }
    buf[py*w + px] = pixel;
}

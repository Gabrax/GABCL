typedef struct {
    int type;           
    float x, y;          
    float size;          
    float angle;
    unsigned int color; 
} Shape;

inline uint draw_square(uint pixel, const Shape shape, const int px, const int py)
{
    // center of square
    float cx = shape.x + shape.size / 2.0f;
    float cy = shape.y + shape.size / 2.0f;

    // pixel relative to center
    float dx = px - cx;
    float dy = py - cy;

    // rotate pixel coordinates by -angle
    float cosA = cos(-shape.angle);
    float sinA = sin(-shape.angle);
    float rx = dx * cosA - dy * sinA;
    float ry = dx * sinA + dy * cosA;

    // check bounds relative to center
    if (rx >= -shape.size/2.0f && rx < shape.size/2.0f &&
        ry >= -shape.size/2.0f && ry < shape.size/2.0f)
        pixel = shape.color;

    return pixel;
}

inline uint draw_circle(uint pixel, const Shape shape, const int px, const int py)
{
    int cx = shape.x + shape.size / 2;
    int cy = shape.y + shape.size / 2;
    int dx = px - cx;
    int dy = py - cy;
    int r = shape.size / 2;
    if (dx * dx + dy * dy <= r * r) pixel = shape.color;
    return pixel;
}

inline uint draw_triangle(uint pixel, const Shape shape, const int px, const int py)
{
    // center of triangle
    float cx = shape.x + shape.size / 2.0f;
    float cy = shape.y + shape.size / 2.0f;

    // pixel relative to center
    float dx = px - cx;
    float dy = py - cy;

    // rotate pixel coordinates by -angle
    float cosA = cos(-shape.angle);
    float sinA = sin(-shape.angle);
    float rx = dx * cosA - dy * sinA;
    float ry = dx * sinA + dy * cosA;

    // translate back to top-left coordinates
    float tx = rx + shape.size / 2.0f;
    float ty = ry + shape.size / 2.0f;

    int halfBase = shape.size / 2;
    if (ty >= 0 && ty < shape.size)
    {
        int left = (halfBase * ty) / shape.size;
        if (tx >= halfBase - left && tx <= halfBase + left)
            pixel = shape.color;
    }
    return pixel;
}

__kernel void clear_buffer(__global uint* buf, const uint color, const int width, const int height)
{
  int px = get_global_id(0);
  int py = get_global_id(1);

  if (px >= width || py >= height) return;

  buf[py * width + px] = color;
}

__kernel void draw_shapes(__global uint* buf, __global const Shape* shapes, const int numShapes, const int render_width, const int render_height) {
  int px = get_global_id(0);
  int py = get_global_id(1);

  if (px >= render_width || py >= render_height) return;

  uint pixel = 0;

  for (int s = 0; s < numShapes; ++s) {
      Shape sh = shapes[s];
      if (sh.type == 0) {        // square
          pixel = draw_square(pixel, sh, px, py);
      } else if (sh.type == 1) { // circle
          pixel = draw_circle(pixel, sh, px, py);
      } else if (sh.type == 2) { // triangle
          pixel = draw_triangle(pixel, sh, px, py);
      }
  }

  buf[py * render_width + px] = pixel;
}

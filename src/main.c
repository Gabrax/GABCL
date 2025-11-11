#include <stdio.h>
#include "shapes.h"
#include "camera.h"
#include "engine.h"

int main()
{
  InitWindow(800, 600, "GABCL");
  SetTargetFPS(60);
  DisableCursor();

  CustomCamera camera = {0};
  camera_init(&camera,GetScreenWidth(),GetScreenHeight(),90.0f,0.01f,1000.0f);

  CustomModel model = {0};
  custom_model_load(&model, "res/bunny.obj","res/Texture.png",(Color){0,255,0,255});
  custom_model_set_transform(&model, MatTransform((f3){0.0f, 0.0f, 2.0f},
                                                 (f3){0.0f, 0.0f, 0.0f},
                                                 (f3){1.5f, 1.5f, 1.5f}));
  /*custom_model_print_data(&mesh);*/

  Engine engine = {0};
  engine_init(&engine, &camera, &model,"src/shapes.cl",GetScreenWidth(),GetScreenHeight());
  engine_background_color(&engine, (Color){0,0,0,255});

  while (!WindowShouldClose())
  {
    if (IsKeyDown(KEY_W)) camera_process_keys(&camera, FORWARD);
    if (IsKeyDown(KEY_S)) camera_process_keys(&camera, BACKWARD);
    if (IsKeyDown(KEY_A)) camera_process_keys(&camera, LEFT);
    if (IsKeyDown(KEY_D)) camera_process_keys(&camera, RIGHT);

    camera_update(&camera, GetMouseX(), GetMouseY(), true);
    engine_send_camera_matrix(&engine, &camera);

    engine_clear_background(&engine);

    engine_run_rasterizer(&engine, &model);

    engine_read_and_display(&engine);
  }

  engine_close(&engine);

  return 0;
}


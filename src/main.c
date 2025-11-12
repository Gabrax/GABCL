#include "engine.h"

int main()
{
  InitWindow(800, 600, "GABCL");
  SetTargetFPS(60);
  DisableCursor();

  CustomCamera camera = {0};
  engine_init_camera(&camera,GetScreenWidth(),GetScreenHeight(),90.0f,0.01f,1000.0f);

  CustomModel model = {0};
  engine_load_model(&model, "res/bunny.obj",NULL,(Color){0,255,0,255});
  engine_set_model_transform(&model, MatTransform((f3){0.0f, 0.0f, 2.0f},
                                                 (f3){0.0f, 0.0f, 0.0f},
                                                 (f3){1.5f, 1.5f, 1.5f}));

  Engine engine = {0};
  engine_init(&engine, &camera, &model,"src/shapes.cl",GetScreenWidth(),GetScreenHeight());
  engine_background_color(&engine, (Color){0,0,0,255});

  while (!WindowShouldClose())
  {
    if (IsKeyDown(KEY_W)) engine_process_camera_keys(&camera, FORWARD);
    if (IsKeyDown(KEY_S)) engine_process_camera_keys(&camera, BACKWARD);
    if (IsKeyDown(KEY_A)) engine_process_camera_keys(&camera, LEFT);
    if (IsKeyDown(KEY_D)) engine_process_camera_keys(&camera, RIGHT);

    engine_update_camera(&camera, GetMouseX(), GetMouseY(), true);
    engine_send_camera_matrix(&engine, &camera);

    engine_clear_background(&engine);

    engine_run_rasterizer(&engine, &model);

    engine_read_and_display(&engine);
  }

  engine_close(&engine);

  return 0;
}


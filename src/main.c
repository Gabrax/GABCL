#include "engine.h"

int main()
{
  InitWindow(800, 600, "GABCL");
  /*SetTargetFPS(60);*/
  DisableCursor();

  CustomCamera camera = {0};
  engine_init_camera(&camera,GetScreenWidth(),GetScreenHeight(),90.0f,0.01f,1000.0f);

  Engine engine = {0};
  engine_init(&engine, &camera, "src/shapes.cl",GetScreenWidth(),GetScreenHeight());
  engine_background_color(&engine, (Color){0,0,0,255});

  CustomModel model = {0};
  engine_load_model(&model, "res/slime.obj","res/Texture.png",(Color){0,255,0,255},MatTransform((f3){0.0f, 0.0f, 5.0f},(f3){0.0f, 0.0f, 0.0f},(f3){1.5f, 1.5f, 1.5f}));

  CustomModel model2 = {0};
  engine_load_model(&model2, "res/bunny.obj",NULL,(Color){0,255,0,255},MatTransform((f3){3.0f, 0.0f, 5.0f},(f3){0.0f, 0.0f, 0.0f},(f3){1.5f, 1.5f, 1.5f}));

  engine_upload_models_data(&engine);

  while (!WindowShouldClose())
  {
    engine_clear_background(&engine);

    if (IsKeyDown(KEY_W)) engine_process_camera_keys(&camera, FORWARD);
    if (IsKeyDown(KEY_S)) engine_process_camera_keys(&camera, BACKWARD);
    if (IsKeyDown(KEY_A)) engine_process_camera_keys(&camera, LEFT);
    if (IsKeyDown(KEY_D)) engine_process_camera_keys(&camera, RIGHT);

    engine_update_camera(&camera, GetMouseX(), GetMouseY(), true);
    engine_send_camera_matrix(&engine, &camera);

    engine_run_rasterizer(&engine);
    engine_read_and_display(&engine);
  }

  engine_close(&engine);

  return 0;
}


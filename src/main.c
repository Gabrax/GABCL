#include "engine.h"
#include "raylib.h"

int main()
{
  InitWindow(800, 600, "GABCL");
  SetTargetFPS(60);
  DisableCursor();

  engine_init("src/shapes.cl",GetScreenWidth(),GetScreenHeight());
  engine_background_color((Color){0,0,0,255});

  engine_init_camera(GetScreenWidth(),GetScreenHeight(),90.0f,0.01f,1000.0f);

  engine_load_model("res/rayman_2_mdl.obj","res/Rayman.png",MatTransform((f3){1.0f, 0.0f, 3.0f},(f3){0.0f, 180.0f, 0.0f},(f3){0.1f, 0.1f, 0.1f}));
  /*CustomModel model2 = {0};*/
  /*engine_load_model(&model2, "res/bunny.obj",NULL,(Color){0,255,0,255},MatTransform((f3){1.0f, 0.0f, 3.0f},(f3){0.0f, 180.0f, 0.0f},(f3){1.1f, 1.1f, 1.1f}));*/

  engine_upload_models_data();

  while (!WindowShouldClose())
  {
    engine_clear_background();

    if (IsKeyDown(KEY_W)) engine_process_camera_keys(FORWARD);
    if (IsKeyDown(KEY_S)) engine_process_camera_keys(BACKWARD);
    if (IsKeyDown(KEY_A)) engine_process_camera_keys(LEFT);
    if (IsKeyDown(KEY_D)) engine_process_camera_keys(RIGHT);

    engine_update_camera(GetMouseX(), GetMouseY(), true);
    engine_send_camera_matrix();

    engine_run_rasterizer();
    engine_read_and_display();
    printf("%d\n",GetFPS());
  }

  engine_free_all_models();
  engine_close();

  return 0;
}


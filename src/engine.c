#include "engine.h"

#define GABMATH_IMPLEMENTATION
#include "gab_math.h"
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

typedef struct {
    int triangleOffset;
    int triangleCount;
    int vertexOffset;
    int vertexCount;
    int pixelOffset;
    int texWidth;
    int texHeight;
    f4x4 transform;
} CustomModelGPU;

static CustomModel* models = NULL;
static Triangle* allTriangles = NULL;
static Color* allTexturePixels = NULL;
static CustomModelGPU* gpuModels = NULL;

static size_t totalTriangles = 0;
static size_t totalTexturePixels = 0;

static size_t triOffset = 0;
static size_t pixOffset = 0;

inline const char* loadKernel(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if(!f) { printf("Cannot open kernel file.\n"); return NULL; }
    fseek(f,0,SEEK_END);
    size_t size = ftell(f);
    fseek(f,0,SEEK_SET);
    char* src = (char*)malloc(size+1);
    fread(src,1,size,f);
    src[size] = '\0';
    fclose(f);
    return src;
}

void engine_init(Engine* engine,CustomCamera* camera,const char* kernel,int width, int height)
{
  clGetPlatformIDs(1, &engine->platform, NULL);
  clGetDeviceIDs(engine->platform, CL_DEVICE_TYPE_GPU, 1, &engine->device, NULL);

  engine->context = clCreateContext(NULL, 1, &engine->device, NULL, NULL, NULL);
  engine->queue = clCreateCommandQueue(engine->context, engine->device, 0, NULL);
  
  const char* kernelSource = loadKernel(kernel); 
  engine->program = clCreateProgramWithSource(engine->context, 1, &kernelSource, NULL, &engine->err);
  if (engine->err != CL_SUCCESS) { printf("Error creating program: %d\n", engine->err); }

  engine->err = clBuildProgram(engine->program, 1, &engine->device, NULL, NULL, NULL);
  if (engine->err != CL_SUCCESS) {
      size_t log_size;
      clGetProgramBuildInfo(engine->program, engine->device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
      char* log = (char*)malloc(log_size);
      clGetProgramBuildInfo(engine->program, engine->device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
      printf("OpenCL build error:\n%s\n", log);
      free(log);
  }

  engine->screen_resolution[0] = width;
  engine->screen_resolution[1] = height;

  engine->clearKernel    = clCreateKernel(engine->program, "clear_buffers", NULL);
  engine->vertexKernel   = clCreateKernel(engine->program, "vertex_kernel", NULL);
  engine->fragmentKernel = clCreateKernel(engine->program, "fragment_kernel", NULL);

  engine->frameBuffer = clCreateBuffer(engine->context, CL_MEM_WRITE_ONLY, 
                                      engine->screen_resolution[0] * engine->screen_resolution[1]
                                              * sizeof(Color), NULL, NULL);
  engine->depthBuffer = clCreateBuffer(engine->context, CL_MEM_READ_WRITE,
                                      sizeof(float) * engine->screen_resolution[0]
                                                    * engine->screen_resolution[1],
                                                    NULL, &engine->err);
  engine->projectionBuffer  = clCreateBuffer(engine->context, CL_MEM_READ_ONLY,
                                            sizeof(f4x4), NULL, &engine->err);
  engine->viewBuffer  = clCreateBuffer(engine->context, CL_MEM_READ_ONLY,
                                      sizeof(f4x4), NULL, &engine->err);
  engine->cameraPosBuffer  = clCreateBuffer(engine->context, CL_MEM_READ_ONLY,
                                           sizeof(f3), NULL, &engine->err);

  clSetKernelArg(engine->clearKernel, 0, sizeof(cl_mem), &engine->frameBuffer);
  clSetKernelArg(engine->clearKernel, 1, sizeof(cl_mem), &engine->depthBuffer);
  clSetKernelArg(engine->clearKernel, 2, sizeof(int), &engine->screen_resolution[0]);
  clSetKernelArg(engine->clearKernel, 3, sizeof(int), &engine->screen_resolution[1]);
  clEnqueueNDRangeKernel(engine->queue, engine->clearKernel, 2, NULL,
                         engine->screen_resolution, NULL, 0, NULL, NULL);

  clSetKernelArg(engine->vertexKernel, 5, sizeof(cl_mem), &engine->projectionBuffer); 
  clSetKernelArg(engine->vertexKernel, 6, sizeof(cl_mem), &engine->viewBuffer); 
  clSetKernelArg(engine->vertexKernel, 7, sizeof(cl_mem), &engine->cameraPosBuffer);
  clSetKernelArg(engine->vertexKernel, 8, sizeof(int), &engine->screen_resolution[0]);
  clSetKernelArg(engine->vertexKernel, 9, sizeof(int), &engine->screen_resolution[1]);

  clSetKernelArg(engine->fragmentKernel, 0, sizeof(cl_mem), &engine->frameBuffer);
  clSetKernelArg(engine->fragmentKernel, 2, sizeof(int), &engine->screen_resolution[0]);
  clSetKernelArg(engine->fragmentKernel, 3, sizeof(int), &engine->screen_resolution[1]);
  clSetKernelArg(engine->fragmentKernel, 4, sizeof(cl_mem), &engine->depthBuffer);
  clSetKernelArg(engine->fragmentKernel, 5, sizeof(cl_mem), &engine->cameraPosBuffer);

  Image img = GenImageColor(engine->screen_resolution[0], engine->screen_resolution[1], engine->clearColor);
  engine->texture = LoadTextureFromImage(img);
  free(img.data); // pixel buffer is managed by OpenCL

  engine->pixelBuffer = (Color*)malloc(engine->screen_resolution[0]
                                       * engine->screen_resolution[1] 
                                       * sizeof(Color));
  
  clEnqueueWriteBuffer(engine->queue, engine->projectionBuffer, CL_TRUE, 0,
                       sizeof(f4x4), &camera->proj, 0, NULL, NULL);
}

void engine_background_color(Engine* engine, Color color)
{
  clSetKernelArg(engine->clearKernel, 4, sizeof(Color), &color);
}

void engine_clear_background(Engine* engine)
{
  clEnqueueNDRangeKernel(engine->queue, engine->clearKernel, 2, NULL, engine->screen_resolution, NULL, 0, NULL, NULL);
}

void engine_send_camera_matrix(Engine* engine, CustomCamera* camera)
{
  clEnqueueWriteBuffer(engine->queue, engine->cameraPosBuffer, CL_TRUE, 0,
                       sizeof(f3), &camera->Position, 0, NULL, NULL);
  clEnqueueWriteBuffer(engine->queue, engine->viewBuffer, CL_TRUE, 0,
                       sizeof(f4x4), &camera->look_at, 0, NULL, NULL);
}

void engine_run_rasterizer(Engine* engine)
{
  clEnqueueNDRangeKernel(engine->queue, engine->vertexKernel, 1, NULL,
                         &totalTriangles, NULL, 0, NULL, NULL);
  clEnqueueNDRangeKernel(engine->queue, engine->fragmentKernel, 2, NULL,
                         engine->screen_resolution, NULL, 0, NULL, NULL);
}

void engine_read_and_display(Engine* engine)
{
  clEnqueueReadBuffer(engine->queue, engine->frameBuffer, CL_TRUE, 0,
                      engine->screen_resolution[0] * engine->screen_resolution[1] * sizeof(Color),
                      engine->pixelBuffer, 0, NULL, NULL);
  
  clFinish(engine->queue);
  clEnqueueReadBuffer(engine->queue, engine->frameBuffer, CL_TRUE, 0,
                      engine->screen_resolution[0] * engine->screen_resolution[1] * sizeof(Color),
                      engine->pixelBuffer, 0, NULL, NULL);

  UpdateTexture(engine->texture, engine->pixelBuffer);
  BeginDrawing();
  DrawTexture(engine->texture, 0, 0, WHITE);
  EndDrawing();
}

void engine_close(Engine* engine)
{
  free(engine->pixelBuffer);

  UnloadTexture(engine->texture);
  CloseWindow();

  clReleaseMemObject(engine->frameBuffer);
  clReleaseMemObject(engine->projectedVertsBuffer);
  clReleaseKernel(engine->vertexKernel);
  clReleaseKernel(engine->fragmentKernel);
  clReleaseProgram(engine->program);
  clReleaseCommandQueue(engine->queue);
  clReleaseContext(engine->context);
}

void engine_load_model(CustomModel* model, const char* filePath,const char* texturePath, Color color,f4x4 transform)
{
    static int seeded = 0;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = 1;
    }

    const struct aiScene* scene = aiImportFile(
        filePath,
        aiProcess_Triangulate |          // convert all faces to triangles
        aiProcess_JoinIdenticalVertices |// merge shared vertices
 aiProcess_GenSmoothNormals |     // generate smooth vertex normals
        aiProcess_ImproveCacheLocality | // better vertex cache usage
        aiProcess_OptimizeMeshes         // reduce draw calls
    );

    if (!scene || scene->mNumMeshes == 0) {
        fprintf(stderr, "Failed to load model: %s\n", filePath);
        aiReleaseImport(scene);
        return;
    }

    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        const struct aiMesh* mesh = scene->mMeshes[m];

        for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
            const struct aiFace* face = &mesh->mFaces[f];
            if (face->mNumIndices != 3) continue; // skip non-triangles

            Triangle tri = {0};
            for (int i = 0; i < 3; i++) {
                unsigned int idx = face->mIndices[i];
                if (mesh->mVertices) {
                    tri.vertex[i].x = mesh->mVertices[idx].x;
                    tri.vertex[i].y = mesh->mVertices[idx].y;
                    tri.vertex[i].z = mesh->mVertices[idx].z;
                }
                if (mesh->mNormals) {
                    tri.normal[i].x = mesh->mNormals[idx].x;
                    tri.normal[i].y = mesh->mNormals[idx].y;
                    tri.normal[i].z = mesh->mNormals[idx].z;
                }
                if (mesh->mTextureCoords[0]) {
                    tri.uv[i].x = mesh->mTextureCoords[0][idx].x;
                    tri.uv[i].y = mesh->mTextureCoords[0][idx].y;
                }
            }
            Color c;
            c.r = rand() % 256; // 0-255
            c.g = rand() % 256;
            c.b = rand() % 256;
            c.a = 255;           // fully opaque
            tri.color = c;
            arrpush(model->triangles, tri);
        }
    }

    model->numTriangles = arrlen(model->triangles);
    model->numVertices = model->numTriangles * 3;
    arrpush(models,*model);
    aiReleaseImport(scene);

  if(texturePath)
  {
    Image img = LoadImage(texturePath);
    model->texWidth = img.width;
    model->texHeight = img.height;
    model->pixels = (Color*)img.data;
    /*UnloadImage(img);*/
  }

  model->transform = transform;


  for (size_t t = 0; t < model->numTriangles; t++) {
      arrpush(allTriangles, model->triangles[t]);
  }

  size_t numPixels = model->texWidth * model->texHeight;
  for (size_t p = 0; p < numPixels; p++) {
      arrpush(allTexturePixels, model->pixels[p]);
  }

  CustomModelGPU m;
  m.triangleOffset = triOffset;
  m.triangleCount  = (int)model->numTriangles;
  m.vertexOffset   = 0;
  m.vertexCount    = 0;
  m.pixelOffset    = pixOffset;
  m.texWidth       = model->texWidth  > 0 ? model->texWidth  : 0;
  m.texHeight      = model->texHeight > 0 ? model->texHeight : 0;
  m.transform      = transform;
  arrpush(gpuModels, m);

  triOffset += model->numTriangles;
  pixOffset += numPixels;

  totalTriangles += model->numTriangles;
  totalTexturePixels += model->texWidth * model->texHeight;
}

void engine_upload_models_data(Engine* engine)
{  
  int numModels = arrlen(gpuModels);
  totalTriangles *= 3;

  engine->trianglesBuf = clCreateBuffer(engine->context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        arrlen(allTriangles) * sizeof(Triangle), allTriangles, &engine->err);

  engine->pixelsBuf = clCreateBuffer(engine->context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        arrlen(allTexturePixels) * sizeof(Color), allTexturePixels, &engine->err);

  engine->modelsBuf = clCreateBuffer(engine->context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        arrlen(gpuModels) * sizeof(CustomModelGPU), gpuModels, &engine->err);

  engine->projectedVertsBuffer = clCreateBuffer(engine->context, CL_MEM_READ_WRITE,
                                          sizeof(f4) * totalTriangles, NULL, NULL);

  clSetKernelArg(engine->vertexKernel, 4, sizeof(cl_mem), &engine->projectedVertsBuffer);
  clSetKernelArg(engine->fragmentKernel, 1, sizeof(cl_mem), &engine->projectedVertsBuffer);

  clSetKernelArg(engine->vertexKernel, 0, sizeof(cl_mem), &engine->trianglesBuf);
  clSetKernelArg(engine->vertexKernel, 1, sizeof(cl_mem), &engine->modelsBuf);
  clSetKernelArg(engine->vertexKernel, 2, sizeof(int), &numModels);
  clSetKernelArg(engine->vertexKernel, 3, sizeof(int), &totalTriangles);

  clSetKernelArg(engine->fragmentKernel, 6, sizeof(cl_mem), &engine->trianglesBuf);
  clSetKernelArg(engine->fragmentKernel, 7, sizeof(cl_mem), &engine->modelsBuf);
  clSetKernelArg(engine->fragmentKernel, 8, sizeof(int), &numModels);
  clSetKernelArg(engine->fragmentKernel, 9, sizeof(int), &totalTriangles);
  clSetKernelArg(engine->fragmentKernel, 10, sizeof(cl_mem), &engine->pixelsBuf);
}

void engine_set_model_transform(CustomModel* model,f4x4 transform)
{
  model->transform = transform;
}

void engine_print_model_data(const CustomModel* model)
{
  for (int i = 0; i < model->numTriangles; i++) {
      const Triangle* t = &model->triangles[i];
      printf("Triangle %d:\n", i);

      for (int v = 0; v < 3; v++) {
          printf("  Vertex %d:\n", v);
          printf("    Position: (%f, %f, %f)\n",
                 t->vertex[v].x,
                 t->vertex[v].y,
                 t->vertex[v].z);
          printf("    UV:        (%f, %f)\n",
                 t->uv[v].x,
                 t->uv[v].y);
          printf("    Normal:    (%f, %f, %f)\n",
                 t->normal[v].x,
                 t->normal[v].y,
                 t->normal[v].z);
      }

      printf("  Color: (r=%d g=%d b=%d a=%d)\n\n",
             t->color.r, t->color.g, t->color.b, t->color.a);
  }
}

void engine_free_model(CustomModel* model)
{
  arrfree(model->triangles);
  model->triangles = NULL;
  model->numTriangles = 0;
}

void engine_init_camera(CustomCamera* camera, int width, int height, float fov, float near_plane, float far_plane)
{
  camera->Position = (f3){0.0f, 0.0f, 0.0f};
  camera->WorldUp = (f3){0.0f, 1.0f, 0.0f};
  camera->Front = (f3){0.0f, 0.0f, 1.0f};
  camera->aspect_ratio = (float)width / (float)height;
  camera->near_plane = near_plane;
  camera->far_plane = far_plane;
  camera->fov = fov;
  camera->fov_rad = DegToRad(camera->fov);  
  camera->proj = MatPerspective(camera->fov_rad, camera->aspect_ratio, camera->near_plane, camera->far_plane);
  camera->look_at = MatLookAt(camera->Position, f3Add(camera->Position, camera->Front), camera->WorldUp);
  camera->yaw = 90.0f;
  camera->pitch = 0.0f;
  camera->speed = 2.0f;
  camera->sens = 0.1f;
  camera->lastX = width / 2.0f;
  camera->lastY = height / 2.0f;
  camera->firstMouse = true;
  camera->deltaTime = 1.0/60.0f;
}

void engine_process_camera_keys(CustomCamera* camera, Camera_Movement direction)
{
  float velocity = camera->speed * camera->deltaTime;

  if (direction == FORWARD) camera->Position = f3Add(camera->Position, f3MulS(camera->Front, velocity));
  if (direction == BACKWARD) camera->Position = f3Add(camera->Position, f3MulS(camera->Front, -velocity));
  if (direction == LEFT) camera->Position = f3Add(camera->Position, f3MulS(camera->Right, velocity));
  if (direction == RIGHT) camera->Position = f3Add(camera->Position, f3MulS(camera->Right, -velocity));
}
void engine_update_camera(CustomCamera* camera, float mouseX, float mouseY, bool constrainPitch)
{
  float xoffset, yoffset;
  if (camera->firstMouse)
  {
      camera->lastX = mouseX;
      camera->lastY = mouseY;
      camera->firstMouse = false;
  }

  xoffset = camera->lastX - mouseX;
  yoffset = camera->lastY - mouseY; 

  camera->lastX = mouseX;
  camera->lastY = mouseY;

  xoffset *= camera->sens;
  yoffset *= camera->sens;

  camera->yaw   += xoffset;
  camera->pitch += yoffset;

  if (constrainPitch)
  {
      if (camera->pitch > 89.0f)  camera->pitch = 89.0f;
      if (camera->pitch < -89.0f) camera->pitch = -89.0f;
  }

  f3 front;
  front.x = cosf(DegToRad(camera->yaw)) * cosf(DegToRad(camera->pitch));
  front.y = sinf(DegToRad(camera->pitch));
  front.z = sinf(DegToRad(camera->yaw)) * cosf(DegToRad(camera->pitch));
  camera->Front = f3Norm(front);

  camera->Right = f3Norm(f3Cross(camera->Front, camera->WorldUp));
  camera->Up    = f3Norm(f3Cross(camera->Right, camera->Front));

  camera->look_at = MatLookAt(camera->Position, f3Add(camera->Position, camera->Front), camera->Up);
}

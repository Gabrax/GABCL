#pragma once

#include "Camera.h"
#include "Texture.h"
#include "Buffer.h"
#include "DeltaTime.hpp"
#include "ModelManager.h"

struct Renderer
{
	static void Init();
	static void Shutdown();

  static void DrawScene(DeltaTime& dt, const std::function<void()>& geometry, const std::function<void()>& lights);
	static void BeginScene(const Camera& camera);
	static void EndScene();

  static void BakeSkyboxTextures(const std::string& name,const std::shared_ptr<Texture>& texture);
  static void DrawSkybox(const std::string& name);

  static void AddDrawCommand(const std::string& modelName, uint32_t verticesSize, uint32_t indicesSize);
  static void RebuildDrawCommandsForModel(const std::shared_ptr<Model>& model, bool render);
  static void InitDrawCommandBuffer();

  static void DrawFullscreenQuad();
  static void SetFullscreen(const std::string& sound, bool windowed);
  static void SwitchRenderState();

private:

	static void StartBatch();
	static void Flush();
	static void NextBatch();
	static void LoadShaders();

  static void DrawFramebuffer(uint32_t textureID);
	static void DrawEditorFrameBuffer(uint32_t framebufferTexture);
  static uint32_t GetActiveWidgetID();
	static void BlockEvents(bool block);
	static bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);
};

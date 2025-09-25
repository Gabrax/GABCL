#include "engine.h"

#include "backend/BackendLogger.h"
#include "backend/LayerStack.h"
#include "backend/LightManager.h"
#include "backend/ModelManager.h"
#include "backend/Renderer.h"

Engine* Engine::s_Instance = nullptr;

Engine::Engine()
{
  s_Instance = this;
  Run();
}

Engine::~Engine() = default;

void Engine::Run()
{
  Logger::Init();

  m_Window = Window::Create({ "GABAMI", 1000, 600 });

  LightManager::Init();
	Renderer::Init();
  ModelManager::Init();

  /*Application* m_App = new Application;*/
  /*LayerStack::PushLayer(m_App);*/

  while(m_Window->IsRunning())
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    DeltaTime dt;

    /*if (!m_Window->IsMinimized()) LayerStack::OnUpdate(dt);*/

    m_Window->Update();
  }
}



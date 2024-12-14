#include "Engine.h"
#include "Backend/BackEnd.h"

void Engine::Run()
{
    BackEnd::Init();

    while (BackEnd::WindowIsOpen()) {

        BackEnd::BeginFrame();
        BackEnd::UpdateSubSystems();


       
        
    }

    BackEnd::CleanUp();
}

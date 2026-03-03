#include "Application.h"
#include "LuaBehaviour.h"
#include "ScriptComponent.h"

#include "Renderer.h"

#include <chrono>
#include <iostream>

using namespace Apex::Windowing;
using namespace Apex::Rendering;
using namespace Apex::Scripting;

void Apex::Application::Run()
{
    Init();

    while (!m_window->ShouldClose())
    {   
        Tick();
    }

    Shutdown();
}

void Apex::Application::Init()
{
    std::cout << "Application Init\n";


    m_luaManager = std::make_unique<LuaManager>();

    m_window = std::unique_ptr<IWindow>(CreateWindow("Apex Renderer", 1280, 720));

    m_objects.push_back(std::make_unique<Apex::Data::Object>());
    auto& obj = *m_objects.back();

    m_objects.push_back(std::make_unique<Apex::Data::Object>());
    auto& obj2 = *m_objects.back();
    
    auto& script = obj.AddComponent<Apex::Scripting::ScriptComponent>(*m_luaManager);
    script.SetScript("../../Assets/Scripts/ScriptParent.lua");

    auto& script2 = obj2.AddComponent<Apex::Scripting::ScriptComponent>(*m_luaManager);
    script2.SetScript("../../Assets/Scripts/ScriptChild.lua");

    lua_State* L = m_luaManager->GetState();

    // push parent instance
    lua_rawgeti(L, LUA_REGISTRYINDEX, script.GetInstanceRef());

    // push child instance
    lua_rawgeti(L, LUA_REGISTRYINDEX, script2.GetInstanceRef());

    // parent.other = child
    lua_setfield(L, -2, "other");

    lua_pop(L, 1); // pop parent

    obj.Start();
    obj2.Start();
}

void Apex::Application::Shutdown()
{
    std::cout << "Application Shutdown\n" << std::endl;
}

void Apex::Application::Tick()
{
    m_window->PollEvents();

    m_deltaTime = m_window->GetDeltaTime();
    m_totalTime += m_deltaTime;

    ProcessInput();

    Update();

    // TODO: m_physicsSystem->Step(dt);

    SyncPhysicsToTransforms();

    Render();

    m_window->SwapBuffers();
}

void Apex::Application::ProcessInput()
{
    // Camera keyboard movement
    //m_camera->ProcessKeyboard(
    //    m_window->IsKeyPressed(Key::W),
    //    m_window->IsKeyPressed(Key::S),
    //    m_window->IsKeyPressed(Key::A),
    //    m_window->IsKeyPressed(Key::D),
    //    m_window->IsKeyPressed(Key::E),
    //    m_window->IsKeyPressed(Key::Q),
    //    m_deltaTime
    //);
}

void Apex::Application::Update()
{
    for (auto& obj : m_objects)
        obj->Update(m_deltaTime);
}

void Apex::Application::SyncPhysicsToTransforms()
{
    // TODO: Synchronize scene objects transforms after physics simulation
}

void Apex::Application::Render()
{        
    // Clear
    //m_rhi->Clear(0.12f, 0.12f, 0.15f);

    //// Matrices from camera
    //LibMath::Matrix4 view = m_camera->GetViewMatrix();
    //LibMath::Matrix4 proj = m_camera->GetProjectionMatrix(m_window->GetAspectRatio(), m_camera->m_fov);

    //// Draw cube
    //cubeRenderer->Render(view, proj, m_camera->GetPosition(), m_totalTime);
}
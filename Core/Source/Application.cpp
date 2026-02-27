#include "Application.h"
#include "LuaBehaviour.h"

#include "Renderer.h"

#include <chrono>
#include <iostream>

using namespace Apex::Windowing;
using namespace Apex::Rendering;

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

    //using namespace Apex::Scripting;

    //m_luaManager = std::make_unique<LuaManager>();
    //int scriptRef = m_luaManager->LoadScript("../../Assets/Scripts/Script.lua");

    //int instance1 = m_luaManager->CreateScriptInstance(scriptRef);
    //int instance2 = m_luaManager->CreateScriptInstance(scriptRef);

    //m_objects.push_back(std::make_unique<LuaBehaviour>(m_luaManager.get(), instance1));
    //m_objects.push_back(std::make_unique<LuaBehaviour>(m_luaManager.get(), instance2));

    //for (auto& obj : m_objects)
    //    obj->Start();

    m_window = std::unique_ptr<IWindow>(CreateWindow("Apex Renderer", 1280, 720));
    m_rhi = std::unique_ptr<IRHI>(CreateOpenGLRHI());
    m_camera = std::make_unique<Camera>();

    m_window->SetCursorCaptured(true);

    // Input callbacks
    m_window->SetKeyCallback([&](Key key, bool pressed)
        {
            if (key == Key::Escape && pressed)
                m_window->Close();
        });

    m_window->SetMouseMoveCallback([&](double dx, double dy)
        {
            m_camera->ProcessMouse(dx, dy);
        });

    m_window->SetScrollCallback([&](double, double yoff)
        {
            m_camera->ProcessScroll(yoff);
        });

    // Scene objects
    cubeRenderer = std::make_unique<Renderer>(m_rhi.get());
    cubeRenderer->LoadTexture("../../Assets/Textures/container.jpg");
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
    m_camera->ProcessKeyboard(
        m_window->IsKeyPressed(Key::W),
        m_window->IsKeyPressed(Key::S),
        m_window->IsKeyPressed(Key::A),
        m_window->IsKeyPressed(Key::D),
        m_window->IsKeyPressed(Key::E),
        m_window->IsKeyPressed(Key::Q),
        m_deltaTime
    );
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
    m_rhi->Clear(0.12f, 0.12f, 0.15f);

    // Matrices from camera
    LibMath::Matrix4 view = m_camera->GetViewMatrix();
    LibMath::Matrix4 proj = m_camera->GetProjectionMatrix(m_window->GetAspectRatio(), m_camera->m_fov);

    // Draw cube
    cubeRenderer->Render(view, proj, m_camera->GetPosition(), m_totalTime);
}
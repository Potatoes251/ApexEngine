#include "Application.h"
#include "LuaBehaviour.h"

#include <chrono>
#include <iostream>

void Apex::Application::Run()
{
    Init();

    auto lastTime = std::chrono::high_resolution_clock::now();

    while (m_running)
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = currentTime - lastTime;

        float deltaTime = elapsed.count();
        lastTime = currentTime;

        Tick(deltaTime);
    }

    Shutdown();
}

void Apex::Application::Init()
{
    std::cout << "Application Init\n";

    using namespace Apex::Scripting;

    m_luaManager = std::make_unique<LuaManager>();
    int scriptRef = m_luaManager->LoadScript("../../Assets/Scripts/Script.lua");

    int instance1 = m_luaManager->CreateScriptInstance(scriptRef);
    int instance2 = m_luaManager->CreateScriptInstance(scriptRef);

    m_objects.push_back(std::make_unique<LuaBehaviour>(m_luaManager.get(), instance1));
    m_objects.push_back(std::make_unique<LuaBehaviour>(m_luaManager.get(), instance2));

    for (auto& obj : m_objects)
        obj->Start();
}

void Apex::Application::Shutdown()
{
    std::cout << "Application Shutdown\n" << std::endl;
}

void Apex::Application::Tick(float dt)
{
    ProcessInput();

    Update(dt);

    // TODO: m_physicsSystem->Step(dt);

    SyncPhysicsToTransforms();

    Render();
}

void Apex::Application::ProcessInput()
{
    // TODO: Handle window events and input system
}

void Apex::Application::Update(float dt)
{
    for (auto& obj : m_objects)
        obj->Update(dt);
}

void Apex::Application::SyncPhysicsToTransforms()
{
    // TODO: Synchronize scene objects transforms after physics simulation
}

void Apex::Application::Render()
{
    // TODO: Add rendering system here when implemented
}
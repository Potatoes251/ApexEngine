#include "Application.h"

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
    std::cout << "Application Init\n" << std::endl;
    
    // TODO: crate objects and call Start() on scene
}

void Apex::Application::Shutdown()
{

    std::cout << "Application Shutdown\n" << std::endl;
}

void Apex::Application::Tick(float dt)
{
    ProcessInput();
    Update(dt);
    Render();
}

void Apex::Application::ProcessInput()
{
    // TODO: Handle window events and input system
}

void Apex::Application::Update(float dt)
{
    // TODO: Update scene
}

void Apex::Application::Render()
{
    // TODO: Add rendering system here when implemented
}
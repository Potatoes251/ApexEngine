#include "Application.h"

#include <chrono>
#include <iostream>

Apex::Application::Application()
{

}

Apex::Application::~Application()
{

}

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

    m_luaManager.LoadScript("../../Assets/Scripts/Script.lua");
    m_luaManager.CallStartAll();
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
    
}

void Apex::Application::Update(float dt)
{
    m_luaManager.CallUpdateAll(dt);
}

void Apex::Application::Render()
{

}
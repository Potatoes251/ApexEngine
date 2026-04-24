#include "Game.h"
#include "Camera.h"

using namespace Apex::Game;
using namespace Apex::UserInterface;

bool Game::Init()
{
    if (InitWindow() == false) return false;

    m_gui = std::unique_ptr<IGUI>(CreateGUI(m_window->NativeHandle()));

    if (!m_gui) return false;

    m_app = std::make_unique<Apex::Application>();
    if (!m_app->Init(m_window.get()))
        return false;

    m_app->LoadScene();
    m_app->GetScene()->Start();

    m_window->SetMouseMoveCallback([this](double x, double y)
        {
            auto cam = m_app->GetScene()->GetMainCamera();
            if (cam) cam->ProcessMouse(x, y);
        });

    return true;
}

bool Game::InitWindow()
{
    m_window = std::unique_ptr<Apex::Windowing::IWindow>(Apex::Windowing::CreateWindow("Apex Editor", 1280, 720));

    if (!m_window) return false;

    m_window->Maximize();

    m_window->SetCursorCaptured(true);

    return true;
}

void Game::Update()
{
    m_window->UpdateDeltaTime();
    Apex::Windowing::PollAllEvents();
    m_app->Tick();

    int width = m_window->GetWidth();
    int height = m_window->GetHeight();

    m_window->Clear();

    Apex::Rendering::RenderPassInfo pass;
    pass.m_camera = m_app->GetScene()->GetMainCamera();
    pass.m_flags = Apex::Rendering::RenderSkybox;
    pass.m_width = width;
    pass.m_height = height;

    Render(pass);
}

void Game::Render(const Apex::Rendering::RenderPassInfo& pass)
{
    m_app->Render(pass);

    m_window->SwapBuffers();
}
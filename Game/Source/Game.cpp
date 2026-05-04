#include "Game.h"
#include "Camera.h"

#include "HUD.h"

#include "Log.h"

using namespace Apex::Game;
using namespace Apex::UserInterface;

bool Game::Init()
{
    if (InitWindow() == false) return false;

    m_gui = std::unique_ptr<IGUI>(CreateGUI(m_window->NativeHandle()));
    if (!m_gui) return false;

    m_app = std::make_unique<Apex::Application>();
    if (!m_app->Init(m_window.get(), m_gui.get()))
        return false;

    m_app->LoadScene();

    m_uiManager = m_app->GetUI();

    if (m_uiManager)
    {
        m_uiManager->ClearAll();
        m_uiManager->ScanHUDs("Assets/");
        int handle = m_uiManager->CreateHUD("GameHUD");
        m_uiManager->ShowHUD(handle);
    }

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
    m_window = std::unique_ptr<Apex::Windowing::IWindow>(Apex::Windowing::CreateWindow("Apex Game", 1280, 720));

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

    Render();
}

void Game::Render()
{
    m_gui->BeginFrame();

    int width = m_window->GetWidth();
    int height = m_window->GetHeight();
    m_window->Clear();

    auto* cam = m_app->GetScene()->GetMainCamera();

    Apex::Rendering::RenderPassInfo pass;
    pass.m_camera = cam;
    pass.m_flags = Apex::Rendering::RenderSkybox;
    pass.m_width = width;
    pass.m_height = height;

    m_app->Render(m_gui.get(), pass);

    m_gui->EndFrame();

    m_window->SwapBuffers();
}
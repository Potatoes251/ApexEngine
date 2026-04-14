#ifdef _WIN32
#include <windows.h>
#undef CreateWindow
#endif

#include "Application.h"
#include "Window.h"
#include "RenderPassInfo.h"
#include "Camera.h"

#include "Game.h"

#include <memory>
#include <filesystem>

static std::filesystem::path GetExeDir()
{
    wchar_t buf[MAX_PATH];
    GetModuleFileNameW(nullptr, buf, MAX_PATH);
    return std::filesystem::path(buf).parent_path();
}

static std::filesystem::path FindProjectRoot(std::filesystem::path dir)
{
    while (dir.has_parent_path())
    {
        if (std::filesystem::exists(dir / ".git"))
            return dir;
        std::filesystem::path parent = dir.parent_path();
        if (parent == dir) break;
        dir = parent;
    }
    return dir;
}

int main2(int argc, char* argv[])
{
    std::filesystem::current_path(FindProjectRoot(GetExeDir()));

    auto window = std::unique_ptr<Apex::Windowing::IWindow>(Apex::Windowing::CreateWindow("Apex Legends", 1280, 720));
    if (!window) return 1;

    auto app = std::make_unique<Apex::Application>();
    if (!app->Init(window.get(), nullptr))
        return 1;

    app->LoadScene();
    app->GetScene()->Start();

    window->SetCursorCaptured(true);

    window->SetMouseMoveCallback([&](double x, double y)
        {
            Apex::Rendering::Camera* cam = app->GetScene()->GetMainCamera();
            if (cam) cam->ProcessMouse(x, y);
        });

    Apex::Rendering::Camera* cam = app->GetScene()->GetMainCamera();
    if (cam)
    {
        float aspect = 1280.f / 720.f;
        cam->SetProjectionMatrix(aspect);
    }

    while (!window->ShouldClose())
    {
        Apex::Windowing::PollAllEvents();
        app->Tick();

        int width = window->GetWidth();
        int height = window->GetHeight();

        window->Clear();

        Apex::Rendering::RenderPassInfo pass;
        pass.m_camera = app->GetScene()->GetMainCamera();
        pass.m_flags = Apex::Rendering::RenderSkybox;
        pass.m_width = width;
        pass.m_height = height;
        app->Render(pass);

        window->SwapBuffers();

        app->GetResourceManager()->ProcessGpuUploads();
    }
    return 0;
}

int main(int argc, char* argv[])
{
    std::filesystem::current_path(GetExeDir());

    Apex::Game::Game game;

    if (!game.Init()) return 1;

    while (!game.GetWindow()->ShouldClose())
    {
        game.Update();
    }

    return 0;
}
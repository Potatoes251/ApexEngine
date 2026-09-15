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
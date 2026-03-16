#include "Editor.h"

#include "Window.h"

#include <filesystem>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
static std::filesystem::path GetExeDir()
{
    wchar_t buf[MAX_PATH];
    GetModuleFileNameW(nullptr, buf, MAX_PATH);
    return std::filesystem::path(buf).parent_path();
}
#else
static std::filesystem::path GetExeDir(const char* argv0)
{
    return std::filesystem::path(argv0).parent_path();
}
#endif

// Walk up from the exe until we find the true project root.
// We identify it by the presence of .git  folder,
// which only exist at the project root, unlike Assets/ which
// may be copied into build output folders.
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

int main(int /*argc*/, char* argv[])
{
#ifdef _WIN32
    std::filesystem::path exeDir = GetExeDir();
#else
    std::filesystem::path exeDir = GetExeDir(argv[0]);
#endif

    std::filesystem::path projectRoot = FindProjectRoot(exeDir);
    std::filesystem::current_path(projectRoot);

    Apex::Editor::Editor editor;
    if (!editor.InitWindow())
        return 1;

    auto app = std::make_unique<Apex::Application>();
    // dont create window for the scene while in editor
    if (!app->Init(false))
        return 1;

	app->LoadScene();

    if (!editor.InitPanels(*app))
        return 1;

    while (!editor.GetWindow().ShouldClose())
    {
        // Processes events for all windows must be unconditional
        // so the editor window stays responsive when the game is stopped.
        Apex::Windowing::PollAllEvents();
        editor.Update();
        // Editor render — always runs while editor window is open.
        editor.Render();
        app->GetResourceManager()->ProcessGpuUploads();
    }

    editor.Shutdown();
    if (app)
        app->Shutdown();

	return 0;
}
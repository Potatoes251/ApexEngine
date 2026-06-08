#include "Editor.h"

#include "Window.h"

#include "ResourceManager.h"

#include <filesystem>
#include <iostream>

#ifdef _WIN32

#include <windows.h>

#endif

static std::filesystem::path GetExeDir(const char* argv0)
{
    return std::filesystem::path(argv0).parent_path();
}

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
    std::filesystem::path exeDir = GetExeDir(argv[0]);

    std::filesystem::path projectRoot = FindProjectRoot(exeDir);
    std::filesystem::current_path(projectRoot);

    Apex::Editor::Editor editor;
    if (!editor.InitWindow())
        return 1;

    Apex::Application app(editor.GetWindow(), editor.GetGUI());

	app.LoadScene(false, false);

    if (!editor.InitPanels(app))
        return 1;

    while (!editor.GetWindow()->ShouldClose())
    {
        // Processes events for all windows must be unconditional
        // so the editor window stays responsive when the game is stopped.
        Apex::Windowing::PollAllEvents();
        editor.Update();
        // Editor render - always runs while editor window is open.
        editor.Render();
        app.GetResourceManager()->ProcessGpuUploads();
    }

    editor.Shutdown();

	return 0;
}
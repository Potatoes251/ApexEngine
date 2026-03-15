#include "Editor.h"

int main(int /*argc*/, char* argv[])
{
    Apex::Editor::Editor editor;
    if (!editor.InitWindow())
        return 1;

    auto app = std::make_unique<Apex::Application>(argv[0]);
    // dont create window for the scene while in editor
    if (!app->Init(false))
        return 1;

	app->LoadScene();

    if (!editor.InitPanels(*app))
        return 1;

    while (!editor.GetWindow().ShouldClose())
    {
        // Processes events for all windows — must be unconditional
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
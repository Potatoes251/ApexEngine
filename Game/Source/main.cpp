#include "Editor.h"

int main(int argc, char* argv[])
{
    Apex::Editor::Editor editor;
    if (!editor.InitWindow())
        return 1;

    auto app = std::make_unique<Apex::Application>(argv[0]);
    if (!app->Init(&editor.GetWindow()))
        return 1;

	app->LoadScene();

    if (!editor.InitPanels(*app))
        return 1;

    while (!editor.GetWindow().ShouldClose())
    {
        // Processes events for all windows � must be unconditional
        // so the editor window stays responsive when the game is stopped.
        Apex::Windowing::PollAllEvents();
        // Game tick � only while the game window is still open.
        if (app && !app->GetWindow().ShouldClose())
        {
            app->Tick(); // context switched inside Tick()
        }
        else if (app)
        {
            editor.OnGameStopped();
            app.reset();
            editor.GetWindow().Focus();
        }

        // Editor render � always runs while editor window is open.
        editor.Render(); 
    }

    editor.Shutdown();
    if (app)
        app->Shutdown();

	return 0;
}
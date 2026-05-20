#include "Game.h"
#include "Camera.h"

#include "HUD.h"

#include "Log.h"

using namespace Apex::Game;
using namespace Apex::UserInterface;

bool Game::Init()
{
    if (!InitWindow()) return false;

    m_gui = std::unique_ptr<IGUI>(CreateGUI(m_window->NativeHandle()));
    if (!m_gui) return false;

    m_app = std::make_unique<Apex::Application>(m_window.get(), m_gui.get());

    m_app->SetSaveRequestedCallback([this]()
        {
            SaveGame();
        });

    UIManager* uiManager = m_app->GetUI();

    if (uiManager)
    {
        uiManager->ScanHUDs("Assets/");
        int handle = uiManager->CreateHUD("GameHUD");
        uiManager->ShowHUD(handle);
    }

    m_app->LoadScene(true);

    m_window->SetMouseMoveCallback([this](double x, double y)
        {
            Rendering::Camera* cam = m_app->GetScene()->GetMainCamera();
            if (cam) cam->ProcessMouse(x, y);
        });

    return true;
}

bool Game::InitWindow()
{
    m_window = std::unique_ptr<Apex::Windowing::IWindow>(Apex::Windowing::CreateWindow("Apex Game", 1280, 720));

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
    pass.m_aspectRatio = m_window->GetAspectRatio();
    pass.m_camera = cam;
    pass.m_width = width;
    pass.m_height = height;

    m_app->Render(m_gui.get(), pass);

    m_gui->EndFrame();

    m_window->SwapBuffers();
}

void Apex::Game::Game::SaveGame()
{
    SaveCurrentGame(0);
}

void Apex::Game::Game::SaveCurrentGame(int slot)
{
    using namespace Apex::Save;

    //auto* app = Apex::Application::Get();
    auto* scene = m_app->GetScene();

    // Locate the player object by name (adjust to your convention).
    Apex::Data::Object* player = nullptr;
    for (auto& obj : scene->GetObjects())
    {
        if (obj->GetName() == "Player")
        {
            player = obj.get();
            break;
        }
    }

    SaveData data;
    data.scenePath = m_app->GetScenePath();

    if (player)
    {
        auto pos = player->GetLocalTransform().getPosition();
        data.playerPosition = { pos[0], pos[1], pos[2] };

        // Retrieve score and lives from a ScriptComponent or a custom
        // PlayerStats component you expose via GetComponent<PlayerStats>().
        // Example (replace with your actual component API):
        //   auto* stats = player->GetComponent<PlayerStats>();
        //   if (stats) { data.score = stats->GetScore(); data.lives = stats->GetLives(); }

        // Collect item IDs: your pickup system should maintain a list of
        // object IDs that were consumed. Example:
        //   for (size_t id : app->GetCollectedItemIds())
        //       data.collectedItems.push_back({ id, "coin" });
    }

    const auto& collected = m_app->GetCollectedObjects();

    for (size_t id : collected)
    {
        data.collectedItems.push_back({ id });
    }

    m_app->GetSaveManager()->Save(data, slot);
}

void Apex::Game::Game::LoadSavedGame(int slot)
{
    using namespace Apex::Save;

    //auto* app = Apex::Application::Get();

    auto optData = m_app->GetSaveManager()->Load(slot);
    if (!optData)
    {
        LOG_WARNING("LoadSavedGame – no data in slot {}", slot);
        return;
    }

    const SaveData& data = *optData;

    // 1. Load the scene without calling Start() yet.
    m_app->LoadScene(data.scenePath, /*startScene=*/false);

    // 2. Remove items the player already collected.
    m_app->GetSaveManager()->ApplyToScene(data, m_app->GetScene());

    // 3. Restore player state.
    auto* scene = m_app->GetScene();
    for (auto& obj : scene->GetObjects())
    {
        if (obj->GetName() != "Player") continue;

        // Restore position via the transform (adjust to your transform API).
        //   obj->GetLocalTransform().setPosition({ data.playerPosition.x,
        //                                          data.playerPosition.y,
        //                                          data.playerPosition.z });

        // Restore score / lives:
        //   auto* stats = obj->GetComponent<PlayerStats>();
        //   if (stats) { stats->SetScore(data.score); stats->SetLives(data.lives); }

        break;
    }

    // 4. Now start the scene (calls Start() on every component).
    m_app->GetScene()->Start();

    LOG_INFO("LoadSavedGame – slot {} loaded (scene: {})", slot, data.scenePath);
}
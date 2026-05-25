#include "Game.h"
#include "Camera.h"

#include "HUD.h"
#include "ScriptComponent.h"

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

    // Try to resume from slot 0; fall back to a fresh scene start
    auto optData = m_app->GetSaveManager()->Load(0);
    if (optData)
    {
        LoadSavedGame(0);
    }
    else
    {
        m_app->LoadScene(true); // no save yet, start fresh
    }

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

    auto* scene = m_app->GetScene();

    Apex::Data::Object* player = nullptr;
    for (auto& obj : scene->GetObjects())
    {
        if (obj->GetName() == "Player")
        {
            player = obj.get();
            break;
        }
    }

    // Start from the existing save data so we don't lose previously collected items
    SaveData data;
    auto existingData = m_app->GetSaveManager()->Load(slot);
    if (existingData)
        data = *existingData;

    data.scenePath = m_app->GetScenePath();

    if (player)
    {
        data.playerPosition = player->GetGlobalPosition();

        auto* script = player->GetComponent<Apex::Scripting::ScriptComponent>();
        if (script && script->GetInstanceRef() != LUA_REFNIL)
        {
            lua_State* L = m_app->GetLuaManager()->GetState();
            lua_rawgeti(L, LUA_REGISTRYINDEX, script->GetInstanceRef());
            lua_getfield(L, -1, "coinCount");
            if (lua_isnumber(L, -1))
                data.score = (int)lua_tonumber(L, -1);
            lua_pop(L, 2); // pop coinCount + instance
        }
    }

    // Merge this session's collected IDs into whatever was already saved
    const auto& collected = m_app->GetCollectedObjects();
    for (size_t id : collected)
    {
        // AddCollectedObject already deduplicates on the app side,
        // but guard here too in case the old save had the same id
        bool alreadyPresent = false;
        for (const auto& item : data.collectedItems)
        {
            if (item.objectId == id) { alreadyPresent = true; break; }
        }
        if (!alreadyPresent)
            data.collectedItems.push_back({ id });
    }

    m_app->GetSaveManager()->Save(data, slot);
}

void Apex::Game::Game::LoadSavedGame(int slot)
{
    using namespace Apex::Save;

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

    // 3. Call Start() on every object of the scene
    m_app->GetScene()->Start();

    // 4. Restore player state.
    auto* scene = m_app->GetScene();
    for (auto& obj : scene->GetObjects())
    {
        if (obj->GetName() != "Player") continue;

        auto* script = obj->GetComponent<Apex::Scripting::ScriptComponent>();
        if (script && script->GetInstanceRef() != LUA_REFNIL)
        {
            lua_State* L = m_app->GetLuaManager()->GetState();
            lua_rawgeti(L, LUA_REGISTRYINDEX, script->GetInstanceRef()); // push instance
            lua_pushinteger(L, data.score);                               // push value
            lua_setfield(L, -2, "coinCount");                             // instance.coinCount = score
            lua_pop(L, 1);                                                // pop instance

            lua_rawgeti(L, LUA_REGISTRYINDEX, script->GetInstanceRef());
            lua_getfield(L, -1, "hud");          // push self.hud
            if (!lua_isnil(L, -1))
            {
                lua_getfield(L, -1, "AddScore"); // push hud:AddScore
                lua_pushvalue(L, -2);            // self (hud)
                lua_pushinteger(L, data.score);
                lua_pcall(L, 2, 0, 0);
            }
            lua_pop(L, 2);
        }

        break;
    }

    LOG_INFO("LoadSavedGame – slot {} loaded (scene: {})", slot, data.scenePath);
}
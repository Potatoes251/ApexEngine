#ifndef APPLICATION
#define APPLICATION

#include "ThreadPool.h"

#include "Scene.h"

#include "RenderPassInfo.h"

#include "SaveManager.h"

#include <functional>
#include <memory>
#include <filesystem>

namespace Apex::Rendering
{
    class IRHI;
}

namespace Apex::Windowing
{
    class IWindow;
}
namespace Apex::Resources
{
    class ResourceManager;
}
namespace Apex::Audio
{
    class IAudioEngine;
}

namespace Apex::UserInterface
{
    class UIManager;
    class IGUI;
}

namespace Apex::Scripting
{
    class LuaManager;
}

namespace Apex::Lighting
{
    class LightManager;
    class ShadowManager;
}

namespace Apex
{        
    class FileLog;

	class Application
	{
    public:
        Application(Windowing::IWindow* window, UserInterface::IGUI* gui);
        ~Application();

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        static Application* Get() { return m_instance; }

        void Tick();
        void LoadScene(bool isBuild, bool startScene = true);
        void LoadScene(const std::string& path, bool startScene = true);

        void SetTargetScene(std::string targetScene) { m_targetScene = targetScene; }

        void Render(UserInterface::IGUI* gui, 
            Rendering::RenderPassInfo pass = Rendering::RenderPassInfo());

        Windowing::IWindow*         GetWindow() { return m_window; }
        Rendering::Scene*           GetScene() { return m_scene.get(); }
        void                        SetScene(std::unique_ptr<Rendering::Scene> scene) { m_scene = std::move(scene); }
        Resources::ResourceManager* GetResourceManager() { return m_resourceManager.get(); }
        Rendering::IRHI*            GetRhi() { return m_rhi.get(); }
        Audio::IAudioEngine*        GetAudio() { return m_audio.get(); }
        UserInterface::UIManager*   GetUI() { return m_uiManager.get(); }
        Scripting::LuaManager*      GetLuaManager() { return m_luaManager.get(); }

        float                       GetDeltaTime() const { return m_deltaTime_s; }

        std::string                 GetPrefsPath() const { return m_prefsPath; }
        const char*                 GetLastSceneKey() const { return m_lastSceneKey; }

        Save::SaveManager*          GetSaveManager() { return m_saveManager.get(); }

        void SetSaveRequestedCallback(std::function<void()> callback) { m_onSaveRequested = std::move(callback); }

        void AddCollectedObject(size_t id);
        const std::vector<size_t>& GetCollectedObjects() const;

        void    Pause();
        void    Unpause();

    private:
        void InitLog();
        void InitWindow(Windowing::IWindow* window);
        void InitResourceManager();
        void InitRHI();
        void InitRendering();
        void InitAudio();
        void InitUI(UserInterface::IGUI* gui);
        void InitLighting();
        void InitLua();

        void UpdateAudioListener();

        static void EnsureDefaultScene(const std::string& path);
        std::string LoadLastScene() const;
        std::string LoadBuildStartScene() const;

        std::function<void()> m_onSaveRequested;

        bool m_running = true;
        float m_deltaTime_s = 0.0f;
        float m_accumulator_s = 0.f;

        std::unique_ptr<Rendering::IRHI> m_rhi;

        std::unique_ptr<Scripting::LuaManager>      m_luaManager;
        std::unique_ptr<Lighting::LightManager>     m_lightManager;
        std::unique_ptr<Lighting::ShadowManager>    m_shadowManager;
        std::unique_ptr<Resources::ResourceManager> m_resourceManager;
        std::unique_ptr<UserInterface::UIManager>   m_uiManager;
        std::unique_ptr<FileLog>					m_fileLog;

        std::vector<size_t> m_collectedObjects;

        Windowing::IWindow* m_window = nullptr;
        std::unique_ptr<Audio::IAudioEngine> m_audio;

        std::unique_ptr<Rendering::Scene> m_scene;

        std::unique_ptr<Save::SaveManager> m_saveManager;

        std::string m_targetScene{};

        static inline Application* m_instance = nullptr;
        
        static constexpr const char* m_buildScenesPath = "Assets/build_scenes.json";
        static constexpr const char* m_prefsPath = "Editor/prefs.ini";
        static constexpr const char* m_lastSceneKey = "LastScene=";
	};
}

#endif // APPLICATION
#ifndef APPLICATION
#define APPLICATION

#include "ThreadPool.h"
#include "Physic.h"
#include "Scene.h"
#include "RenderPassInfo.h"

namespace Apex::Rendering
{
    class IRHI;
    class Scene;
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

#include "Animator.h"

#include <memory>
#include <filesystem>
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
	class Application
	{
    public:
        Application();
        ~Application();

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        bool Init(Windowing::IWindow* window, UserInterface::IGUI* gui);
        void Tick();
        void LoadScene();

        void Render(Rendering::RenderPassInfo pass = Rendering::RenderPassInfo());

        Windowing::IWindow*         GetWindow() { return m_window; }
        Rendering::Scene*           GetScene() { return m_scene.get(); }
        void                        SetScene(std::unique_ptr<Rendering::Scene> scene) { m_scene = std::move(scene); }
        Resources::ResourceManager* GetResourceManager() { return m_resourceManager.get(); }
        Rendering::IRHI*            GetRhi() { return m_rhi.get(); }
        Audio::IAudioEngine*        GetAudio() { return m_audio.get(); }
        UserInterface::UIManager*   GetUI() { return m_uiManager.get(); }
        Scripting::LuaManager*      GetLuaManager() { return m_luaManager.get(); }

    private:
        bool InitWindow(Windowing::IWindow* window);
        bool InitResourceManager();
        bool InitRHI();
        bool InitRendering();
        bool InitAudio();
        bool InitUI(UserInterface::IGUI* gui);
        bool InitLighting();
        bool InitLua();
        bool InitScene();

        void UpdateAudioListener();
        void Update();

        bool m_running = true;
        float m_totalTime = 0.0f;
        float m_deltaTime_s = 0.0f;
        float m_accumulator_s = 0.f;

        const char* m_argv0;

        std::unique_ptr<Rendering::IRHI> m_rhi;

        std::unique_ptr<Scripting::LuaManager> m_luaManager;
        std::unique_ptr<Lighting::LightManager> m_lightManager;
        std::unique_ptr<Lighting::ShadowManager> m_shadowManager;
        std::unique_ptr<Resources::ResourceManager> m_resourceManager;
        std::unique_ptr<UserInterface::UIManager> m_uiManager;

        Windowing::IWindow* m_window = nullptr;
        std::unique_ptr<Audio::IAudioEngine> m_audio;

        Core::ThreadPool m_threadPool;
        std::unique_ptr<Rendering::Scene> m_scene = nullptr;
	};
}

#endif // APPLICATION
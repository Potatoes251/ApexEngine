#ifndef APPLICATION
#define APPLICATION

#include "ThreadPool.h"
#include "Physic.h"


namespace Apex::Rendering
{
    class Scene;
    class IRHI;
    class Camera;
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
        void Run();
        void Tick();
        void LoadScene();
        void Shutdown();
        void ProcessInput();
        void Render(Rendering::Camera* cam = nullptr);

        Windowing::IWindow*         GetWindow() { return m_window; }
        Rendering::Scene*           GetScene() { return m_scene.get(); }
        Resources::ResourceManager* GetResourceManager() { return m_resourceManager.get(); }
        Rendering::IRHI*            GetRhi() { return m_rhi.get(); }
        Audio::IAudioEngine*        GetAudio() { return m_audio.get(); }
        UserInterface::UIManager*   GetUI() { return m_uiManager.get(); }

    private:
        bool InitWindow(Windowing::IWindow* window);
        bool InitResourceManager();
        bool InitCamera();
        bool InitRHI();
        bool InitRendering();
        bool InitAudio();
        bool InitScene();
        bool InitUI(UserInterface::IGUI* gui);
        
        void UpdateAudioListener();
        void Update();
        void SyncPhysicsToTransforms();

    private:
        bool m_running = true;
        float m_totalTime = 0.0f;
        float m_deltaTime = 0.0f;
        float m_accumulator = 0.f;

        std::unique_ptr<Rendering::IRHI> m_rhi;

        std::unique_ptr<Scripting::LuaManager> m_luaManager;
        std::unique_ptr<Resources::ResourceManager> m_resourceManager;
        std::unique_ptr<UserInterface::UIManager> m_uiManager;

        Windowing::IWindow* m_window = nullptr;
        std::unique_ptr<Rendering::Camera> m_camera;
        std::unique_ptr<Audio::IAudioEngine> m_audio;

        Core::ThreadPool m_threadPool;
        std::unique_ptr<Rendering::Scene> m_scene;

        Physic::PhysicSystem m_physics;
	};
}

#endif // APPLICATION
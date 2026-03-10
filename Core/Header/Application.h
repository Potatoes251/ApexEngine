#ifndef APPLICATION
#define APPLICATION

#include "LuaManager.h"
#include "Behaviour.h"
#include "SceneGraph.h"
#include "Object.h"

#include "Window.h"
#include "Audio.h"
#include "RHI.h"
#include "Camera.h"

#include "ThreadPool.h"
#include "ResourceManager.h"
#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"

#include "Physic.h"
#include "BoxCollider.h"
#include "CapsuleCollider.h"
#include "MeshCollider.h"
#include "Rigidbody.h"

#include "DebugRenderer.h"

#include "UI.h"
#include "Scene.h"
#include "Hud.h"

#include <memory>
#include <filesystem>

namespace Apex
{
	class Application
	{
    public:
        Application(const char* argv0) : m_argv0(argv0) {}
        ~Application() = default;

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        bool Init(Apex::Windowing::IWindow* shareContext = nullptr);
        void Run();
        void Tick();
        void LoadScene();
        void Shutdown();

        Apex::Windowing::IWindow& GetWindow() { return *m_window; }

    private:
        bool InitResourceManager();
        bool InitCamera();
        bool InitWindow(Apex::Windowing::IWindow* shareContext);
        bool InitRHI();
        bool InitRendering();
        bool InitAudio();
        bool InitScene();
        bool InitUI();
        
        void ProcessInput();
        void UpdateAudioListener();
        void Update();
        void SyncPhysicsToTransforms();
        void Render();

    private:
        bool m_running = true;
        float m_totalTime = 0.0f;
        float m_deltaTime = 0.0f;
        float m_accumulator = 0.f;

        const char* m_argv0;

        std::unique_ptr<Apex::Rendering::IRHI> m_rhi;

        std::unique_ptr<Apex::Scripting::LuaManager> m_luaManager;
        std::unique_ptr<Apex::Resources::ResourceManager> m_resourceManager;
        std::unique_ptr<Apex::UserInterface::UIManager> m_uiManager;

        std::unique_ptr<Apex::Windowing::IWindow> m_window;
        std::unique_ptr<Apex::Rendering::Camera> m_camera;
        std::unique_ptr<Apex::Audio::IAudioEngine> m_audio;

        Apex::Core::ThreadPool m_threadPool;
        std::unique_ptr<Apex::Rendering::Scene> m_scene;

        Apex::Physic::PhysicSystem m_physics;
	};
}

#endif // APPLICATION
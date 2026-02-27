#ifndef GAME_MANAGER
#define GAME_MANAGER

#include "LuaManager.h"
#include "Behaviour.h"

#include "Window.h"
#include "RHI.h"
#include "Renderer.h"

#include "Camera.h"

#include <memory>

namespace Apex
{
	class Application
	{
    public:
        Application() = default;
        ~Application() = default;

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        void Run();

    private:
        void Init();
        void Shutdown();
        void Tick();
        void ProcessInput();
        void Update();
        void SyncPhysicsToTransforms();
        void Render();

    private:
        bool m_running = true;
        float m_totalTime = 0.0f;
        float m_deltaTime = 0.0f;

        std::unique_ptr<Apex::Scripting::LuaManager> m_luaManager;
        std::vector<std::unique_ptr<Apex::Scripting::Behaviour>> m_objects; // temporary, only for testing

        std::unique_ptr<Apex::Windowing::IWindow> m_window;
        std::unique_ptr<Apex::Rendering::IRHI> m_rhi;
        std::unique_ptr<Apex::Rendering::Camera> m_camera;
        std::unique_ptr<Apex::Rendering::Renderer> cubeRenderer;
	};
}

#endif // GAME_MANAGER
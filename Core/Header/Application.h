#ifndef GAME_MANAGER
#define GAME_MANAGER

#include "LuaManager.h"
#include "Behaviour.h"

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
        void Tick(float dt);
        void ProcessInput();
        void Update(float dt);
        void SyncPhysicsToTransforms();
        void Render();

    private:
        bool m_running = true;

        std::unique_ptr<Apex::Scripting::LuaManager> m_luaManager;

        std::vector<std::unique_ptr<Apex::Scripting::Behaviour>> m_objects; // temporary, only for testing
	};
}

#endif // GAME_MANAGER
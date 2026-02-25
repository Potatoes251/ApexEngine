#ifndef GAME_MANAGER
#define GAME_MANAGER

#include "LuaManager.h"

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
        void Render();

    private:
        bool m_running = true;

        Apex::Scripting::LuaManager m_luaManager;
	};
}

#endif // GAME_MANAGER
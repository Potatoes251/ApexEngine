#ifndef LUA_MANAGER
#define LUA_MANAGER

#include <lua.hpp>

#include <string>
#include <vector>

namespace Apex::UserInterface { class UIManager; }
namespace Apex::Physic { class Collider; }
namespace Apex { class Application; }

namespace Apex::Scripting
{
    class LuaManager
    {
    public:
        LuaManager();
        ~LuaManager();

        LuaManager(const LuaManager& other) = delete;
        LuaManager& operator=(const LuaManager&) = delete;

        lua_State* GetState() { return m_luaState; }

        int LoadScript(const std::string& path);

        void RegisterFunction(const char* name, lua_CFunction func);
        void RegisterFunctions();

        void RegisterHudBindings(Apex::UserInterface::UIManager* ui);
        void RegisterScriptCast(Application* app);

        int CreateScriptInstance(int scriptRef);
        bool CallFunction(int instanceRef, const std::string& funcName);
        bool CallFunction(int instanceRef, const std::string& funcName, float dt);
        bool CallFunction(int instanceRef, const std::string& funcName, Apex::Physic::Collider* collider);

    private:
        lua_State* m_luaState;
        std::vector<int> m_scriptRefs;
    };
}

#endif // LUA_MANAGER
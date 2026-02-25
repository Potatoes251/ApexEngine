#ifndef LUA_MANAGER
#define LUA_MANAGER

#include <lua.hpp>

#include <string>
#include <vector>

namespace Apex::Scripting
{
    class LuaManager
    {
    public:
        LuaManager();
        ~LuaManager();

        int LoadScript(const std::string& path);

        void CallStart(int scriptRef);
        void CallUpdate(int scriptRef, float dt);

        void CallStartAll();
        void CallUpdateAll(float dt);

        void RegisterFunction(const char* name, lua_CFunction func);
        void RegisterFunctions();

    private:
        lua_State* luaState;
        std::vector<int> m_scriptRefs;
    };
}

#endif // LUA_MANAGER
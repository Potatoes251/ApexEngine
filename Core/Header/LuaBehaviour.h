#ifndef LUA_BEHAVIOUR
#define LUA_BEHAVIOUR

#include "Behaviour.h"

#include <lua.hpp>

namespace Apex::Scripting
{
    class LuaBehaviour : public Behaviour
    {
    public:
        LuaBehaviour(lua_State* luaState, int scriptRef)
            : m_luaState(luaState), m_scriptRef(scriptRef) {}
        ~LuaBehaviour();

        LuaBehaviour(const LuaBehaviour&) = delete;
        LuaBehaviour& operator=(const LuaBehaviour&) = delete;

        void Start() override;
        void Update(float dt) override;

    private:
        lua_State* m_luaState = nullptr;
        int m_scriptRef = LUA_NOREF;
        int m_updateFuncRef = LUA_NOREF;
    };
}

#endif // LUA_BEHAVIOUR
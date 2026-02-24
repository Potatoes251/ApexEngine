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
            : L(luaState), m_scriptRef(scriptRef) {}

        ~LuaBehaviour();

        void Start() override;
        void Update(float dt) override;

    private:
        lua_State* L = nullptr;
        int m_scriptRef = LUA_NOREF;
        int m_updateFuncRef = LUA_NOREF;
    };
}

#endif // LUA_BEHAVIOUR
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

        void Start() override;
        void Update(float dt) override;

    private:
        //GameObject* m_owner;
        lua_State* L;
        int m_scriptRef;
    };
}

#endif // LUA_BEHAVIOUR
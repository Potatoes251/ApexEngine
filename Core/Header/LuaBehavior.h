#ifndef LUA_BEHAVIOR
#define LUA_BEHAVIOR

#include "Behavior.h"

#include <lua.hpp>

class LuaBehavior : public Behavior 
{
public:
    LuaBehavior(lua_State* luaState, int scriptRef)
        : L(luaState), m_scriptRef(scriptRef) {}

    void Start() override;
    void Update(float dt) override;

private:
    //GameObject* m_owner;
    lua_State* L;
    int m_scriptRef; 
};

#endif // LUA_BEHAVIOR
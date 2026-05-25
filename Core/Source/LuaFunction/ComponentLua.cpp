#include "LuaFunction/ComponentLua.h"

#include "Component.h"
#include "Object.h"

using namespace Apex;
using namespace Apex::Data;

void Apex::Comp::RegisterLua(Scripting::LuaManager& lua)
{
    lua.RegisterFunction("GetOwner_Internal", Apex::Comp::Lua_GetOwner);
    lua.RegisterFunction("SetEnabled_Internal", Apex::Comp::Lua_SetEnabled);
}

void Apex::Comp::PushLuaObject(lua_State* L, Object* obj)
{
    if (!obj)
    {
        lua_pushnil(L);
        return;
    }

    lua_getglobal(L, "require");
    lua_pushstring(L, "Object");
    lua_pcall(L, 1, 1, 0);

    // call Object:New()
    lua_getfield(L, -1, "New");
    lua_pushvalue(L, -2);
    lua_pcall(L, 1, 1, 0);

    // stack now: instance
    lua_pushlightuserdata(L, obj);
    lua_setfield(L, -2, "__object");

    // remove Object table
    lua_remove(L, -2);
}

int Apex::Comp::Lua_GetOwner(lua_State* L)
{
    Component* comp = GetComponent<Component>(L, 1);
    if (!comp) return 0;

    Object* owner = comp->GetOwner();
    PushLuaObject(L, owner);
    return 1;
}

int Apex::Comp::Lua_SetEnabled(lua_State* L)
{
    Component* comp = GetComponent<Component>(L, 1);
    if (!comp) return 0;

    bool enabled = lua_toboolean(L, 2);

    comp->SetEnabled(enabled);
    return 0;
}

#include "LuaFunction/ComponentLua.h"

#include "Component.h"
#include "Object.h"

using namespace Apex;
using namespace Apex::Data;

void Apex::Comp::RegisterLua(Scripting::LuaManager& lua)
{
    lua.RegisterFunction("GetOwner_Internal", Apex::Comp::Lua_GetOwner);
}

Apex::Component* Apex::Comp::GetComponent(lua_State* L, int index)
{
    lua_getfield(L, index, "__object");
    Component* cc = static_cast<Component*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return cc;
}

void Apex::Comp::PushLuaObject(lua_State* L, Object* obj)
{
    if (!obj)
    {
        lua_pushnil(L);
        return;
    }

    lua_pushlightuserdata(L, obj);
    lua_gettable(L, LUA_REGISTRYINDEX);

    if (lua_isnil(L, -1))
    {
        lua_pop(L, 1);

        lua_newtable(L);
        luaL_getmetatable(L, "Object");
        lua_setmetatable(L, -2);

        lua_pushlightuserdata(L, obj);
        lua_pushvalue(L, -2);
        lua_settable(L, LUA_REGISTRYINDEX);
    }
}

int Apex::Comp::Lua_GetOwner(lua_State* L)
{
    Component* comp = GetComponent(L, 1);
    if (!comp) return 0;

    Object* owner = comp->GetOwner();
    PushLuaObject(L, owner);
    return 1;
}

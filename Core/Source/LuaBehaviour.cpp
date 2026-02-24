#include "LuaBehaviour.h"

#include <lua.hpp>

Apex::Scripting::LuaBehaviour::~LuaBehaviour()
{
    if (m_updateFuncRef != LUA_NOREF)
        luaL_unref(L, LUA_REGISTRYINDEX, m_updateFuncRef);

    luaL_unref(L, LUA_REGISTRYINDEX, m_scriptRef);
}

void Apex::Scripting::LuaBehaviour::Start()
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, m_scriptRef); // push script table

    // --------- STORE UPDATE ---------
    lua_getfield(L, -1, "Update"); // push script.Update

    if (lua_isfunction(L, -1))
    {
        m_updateFuncRef = luaL_ref(L, LUA_REGISTRYINDEX);
    }
    else
    {
        lua_pop(L, 1);
    }

    // --------- CALL START ---------
    lua_getfield(L, -1, "Start");

    if (lua_isfunction(L, -1))
    {
        lua_pushvalue(L, -2); // push self

        if (lua_pcall(L, 1, 0, 0) != LUA_OK)
        {
            printf("Lua Error: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }
    else
    {
        lua_pop(L, 1);
    }

    lua_pop(L, 1); // pop script table
}

void Apex::Scripting::LuaBehaviour::Update(float dt)
{
    if (m_updateFuncRef == LUA_NOREF)
        return;

    lua_rawgeti(L, LUA_REGISTRYINDEX, m_updateFuncRef); // push Update
    lua_rawgeti(L, LUA_REGISTRYINDEX, m_scriptRef);     // push self

    lua_pushnumber(L, dt);

    if (lua_pcall(L, 2, 0, 0) != LUA_OK)
    {
        printf("Lua Error: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
}
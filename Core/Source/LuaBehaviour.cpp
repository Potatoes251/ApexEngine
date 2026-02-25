#include "LuaBehaviour.h"

#include <lua.hpp>

Apex::Scripting::LuaBehaviour::~LuaBehaviour()
{
    if (m_updateFuncRef != LUA_NOREF)
        luaL_unref(m_luaState, LUA_REGISTRYINDEX, m_updateFuncRef);

    if (m_scriptRef != LUA_NOREF)
        luaL_unref(m_luaState, LUA_REGISTRYINDEX, m_scriptRef);
}

void Apex::Scripting::LuaBehaviour::Start()
{
    lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_scriptRef); // push script table

    // --------- STORE UPDATE ---------
    lua_getfield(m_luaState, -1, "Update"); // push script.Update

    if (lua_isfunction(m_luaState, -1))
    {
        m_updateFuncRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
    }
    else
    {
        lua_pop(m_luaState, 1);
    }

    // --------- CALL START ---------
    lua_getfield(m_luaState, -1, "Start");

    if (lua_isfunction(m_luaState, -1))
    {
        lua_pushvalue(m_luaState, -2); // push self

        if (lua_pcall(m_luaState, 1, 0, 0) != LUA_OK)
        {
            printf("Lua Error: %s\n", lua_tostring(m_luaState, -1));
            lua_pop(m_luaState, 1);
        }
    }
    else
    {
        lua_pop(m_luaState, 1);
    }

    lua_pop(m_luaState, 1); // pop script table
}

void Apex::Scripting::LuaBehaviour::Update(float dt)
{
    if (m_updateFuncRef == LUA_NOREF)
        return;

    lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_updateFuncRef); // push Update
    lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_scriptRef);     // push self

    lua_pushnumber(m_luaState, dt);

    if (lua_pcall(m_luaState, 2, 0, 0) != LUA_OK)
    {
        printf("Lua Error: %s\n", lua_tostring(m_luaState, -1));
        lua_pop(m_luaState, 1);
    }
}
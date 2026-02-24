#include "LuaBehavior.h"

#include <lua.hpp>

void LuaBehavior::Start()
{
	lua_rawgeti(L, LUA_REGISTRYINDEX, m_scriptRef);
	lua_getfield(L, -1, "Start");
	lua_pushvalue(L, -2); // self
	lua_pcall(L, 1, 0, 0);
	lua_pop(L, 1);
}

void LuaBehavior::Update(float dt)
{
	lua_rawgeti(L, LUA_REGISTRYINDEX, m_scriptRef);
	lua_getfield(L, -1, "Update");
	lua_pushvalue(L, -2); // self
	lua_pushnumber(L, dt);
	lua_pcall(L, 2, 0, 0);
	lua_pop(L, 1);
}
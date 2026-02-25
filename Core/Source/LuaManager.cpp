#include "LuaManager.h"

#include "Debug.h"
#include "DebugLuaBindings.h"

#include <iostream>

Apex::Scripting::LuaManager::LuaManager()
{
	luaState = luaL_newstate();
	luaL_openlibs(luaState);

	RegisterFunctions();
}

Apex::Scripting::LuaManager::~LuaManager()
{
	for (int ref : m_scriptRefs)
		luaL_unref(luaState, LUA_REGISTRYINDEX, ref);
	lua_close(luaState);
}

int Apex::Scripting::LuaManager::LoadScript(const std::string& path)
{
	if (luaL_dofile(luaState, path.c_str()) != LUA_OK)
	{
		std::cerr << "in LoadScript: " << "Lua error: " << lua_tostring(luaState, -1) << std::endl;
		lua_pop(luaState, 1);
		return -1;
	}

	int ref = luaL_ref(luaState, LUA_REGISTRYINDEX);
	m_scriptRefs.push_back(ref);

	return ref;
}

void Apex::Scripting::LuaManager::CallStart(int scriptRef)
{
	lua_register(luaState, "ApexPrint", Apex::Engine::Debug::Lua_ApexPrint);

	lua_rawgeti(luaState, LUA_REGISTRYINDEX, scriptRef);

	lua_getfield(luaState, -1, "Start");

	if (lua_isfunction(luaState, -1))
	{
		lua_pushvalue(luaState, -2);

		if (lua_pcall(luaState, 1, 0, 0) != LUA_OK)
		{
			std::cerr << "in CallStart: " << "Lua error: " << lua_tostring(luaState, -1) << std::endl;
			lua_pop(luaState, 1);
		}
	}

	lua_pop(luaState, 1);
}

void Apex::Scripting::LuaManager::CallUpdate(int scriptRef, float dt)
{
	lua_rawgeti(luaState, LUA_REGISTRYINDEX, scriptRef);

	lua_getfield(luaState, -1, "Update");

	if (lua_isfunction(luaState, -1))
	{
		lua_pushvalue(luaState, -2);
		lua_pushnumber(luaState, dt);

		if (lua_pcall(luaState, 2, 0, 0) != LUA_OK)
		{
			std::cerr << "in CallUpdate: " << "Lua error: " << lua_tostring(luaState, -1) << std::endl;
			lua_pop(luaState, 1);
		}
	}

	lua_pop(luaState, 1);
}

void Apex::Scripting::LuaManager::CallStartAll()
{
	for (int scriptRef : m_scriptRefs)
	{
		CallStart(scriptRef);
	}
}

void Apex::Scripting::LuaManager::CallUpdateAll(float dt)
{
	for (int scriptRef : m_scriptRefs)
	{
		CallUpdate(scriptRef, dt);
	}
}

void Apex::Scripting::LuaManager::RegisterFunction(const char* name, lua_CFunction func)
{
	lua_register(luaState, name, func);
}

void Apex::Scripting::LuaManager::RegisterFunctions()
{
	RegisterFunction("Print", Apex::Engine::Debug::Lua_ApexPrint);
}

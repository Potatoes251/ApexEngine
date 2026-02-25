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

void Apex::Scripting::LuaManager::RegisterFunction(const char* name, lua_CFunction func)
{
	lua_register(luaState, name, func);
}

void Apex::Scripting::LuaManager::RegisterFunctions()
{
	RegisterFunction("Print", Apex::Engine::Debug::Lua_ApexPrint);
}

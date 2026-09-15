#ifndef LOG_LUA
#define LOG_LUA

#include <lua.hpp>

#include "LuaManager.h"

namespace Apex::Log
{
	void RegisterLua(Scripting::LuaManager& lua);

	int Lua_Log(lua_State* L);
}

#endif // !LOG_LUA


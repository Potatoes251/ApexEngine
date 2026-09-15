#ifndef INPUT_LUA
#define INPUT_LUA

#include <lua.hpp>

#include "InputSystem.h"
#include "LuaManager.h"

namespace Apex::Input
{
	void RegisterLua(Scripting::LuaManager& lua);
	Key StringToKey(char const*);

	int Lua_GetAxis(lua_State* L);
	int Lua_IsKeyDown(lua_State* L);
}

#endif // !INPUT_LUA

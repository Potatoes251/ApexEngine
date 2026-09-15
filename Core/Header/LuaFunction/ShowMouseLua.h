#ifndef SHOW_MOUSE_LUA
#define SHOW_MOUSE_LUA

#include <lua.hpp>

#include "LuaManager.h"

namespace Apex::Mouse
{
	void RegisterLua(Scripting::LuaManager& lua);

	int Lua_ShowMouse(lua_State* L);
	int Lua_HideMouse(lua_State* L);
}


#endif 
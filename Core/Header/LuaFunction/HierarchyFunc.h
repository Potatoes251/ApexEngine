#ifndef HIERARCHY_FUNC
#define HIERARCHY_FUNC

#include "Object.h"

#include <lua.hpp>
#include "LuaManager.h"

namespace Apex::Hierarchy
{
	void RegisterLua(Scripting::LuaManager& lua);

	Apex::Data::Object* GetObject(lua_State* L, int index);
	int Lua_AddChild(lua_State* L);
	int Lua_RemoveChild(lua_State* L);
	int Lua_SetParent(lua_State* L);
}

#endif // HIERARCHY_FUNC
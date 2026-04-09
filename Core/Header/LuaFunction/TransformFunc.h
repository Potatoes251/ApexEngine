#ifndef TRANSFORM_FUNC
#define TRANSFORM_FUNC

#include <lua.hpp>
#include "LuaManager.h"

namespace Apex::Transform
{
	void RegisterLua(Scripting::LuaManager& lua);

	int Lua_SetPosition(lua_State* L);
	int Lua_SetScale(lua_State* L);
}

#endif // TRANSFORM_FUNC

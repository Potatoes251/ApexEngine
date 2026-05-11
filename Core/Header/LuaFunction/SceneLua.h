#ifndef SCENE_LUA
#define SCENE_LUA

#include <lua.hpp>

#include "LuaManager.h"

namespace Apex::Scene
{
	void RegisterLua(Scripting::LuaManager& lua);

	int Lua_Scene_LoadScene(lua_State* L);
}

#endif // !SCENE_LUA

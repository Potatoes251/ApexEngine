#ifndef CAMERA_LUA
#define CAMERA_LUA

#include <lua.hpp>

#include "LuaManager.h"

namespace Apex::Rendering { class Camera; }

namespace Apex::Camera
{
	void RegisterLua(Scripting::LuaManager& lua);

	Rendering::Camera* GetCamera(lua_State* L, int index);

	int Lua_Camera_GetForward(lua_State* L);
	int Lua_Camera_GetRight(lua_State* L);
	int Lua_Camera_GetUp(lua_State* L);

	int Lua_GetCamera(lua_State* L);
}

#endif // !CAMERA_LUA


#ifndef CHARACTER_CONTROLLER_LUA
#define CHARACTER_CONTROLLER_LUA

#include <lua.hpp>
#include "LuaManager.h"

namespace Apex::Controller
{
	class CharacterController;

	void RegisterLua(Scripting::LuaManager& lua);

	CharacterController* GetCC(lua_State* L, int index);

	int Lua_CC_Move(lua_State* L);
	int Lua_CC_Jump(lua_State* L);
	int Lua_CC_GetVelocity(lua_State* L);
	int Lua_CC_IsGrounded(lua_State* L);
	int Lua_CC_IsInWater(lua_State* L);

	int Lua_GetCharacterController(lua_State* L);
}

#endif // !CHARACTER_CONTROLLER_LUA


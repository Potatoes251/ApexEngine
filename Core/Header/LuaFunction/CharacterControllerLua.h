#ifndef CHARACTER_CONTROLLER_LUA
#define CHARACTER_CONTROLLER_LUA

#include <lua.hpp>
#include "LuaManager.h"

namespace Apex::Controller
{
	class CharacterController;
	class AIController;

	void RegisterLua(Scripting::LuaManager& lua);

	int Lua_CC_TP(lua_State* L);
	int Lua_CC_TPToTarget(lua_State* L);
	int Lua_CC_Move(lua_State* L);
	int Lua_CC_Jump(lua_State* L);
	int Lua_CC_GetVelocity(lua_State* L);
	int Lua_CC_GetVerticalVelocity(lua_State* L);
	int Lua_CC_IsGrounded(lua_State* L);
	int Lua_CC_IsInWater(lua_State* L);
	int Lua_AIC_SetTarget(lua_State* L);

	int Lua_GetCharacterController(lua_State* L);
	int Lua_GetAIController(lua_State* L);

	int Lua_CC_SetVelocityX(lua_State* L);
	int Lua_CC_SetVelocityY(lua_State* L);
	int Lua_CC_SetVelocityZ(lua_State* L);
	int Lua_CC_SetVelocity(lua_State* L);
}

#endif // !CHARACTER_CONTROLLER_LUA


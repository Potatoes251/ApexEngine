#ifndef ANIMATION_LUA
#define ANIMATION_LUA

#include <lua.hpp>

#include "LuaManager.h"

namespace Apex::Rendering { class Animator; }

namespace Apex::Anim
{
	void RegisterLua(Scripting::LuaManager& lua);

	int Lua_Anim_Play(lua_State* L);
	int Lua_Anim_Blend(lua_State* L);
	int Lua_Anim_SetBlendRatio(lua_State* L);
	int Lua_Anim_Crossfade(lua_State* L);

	int Lua_GetAnimator(lua_State* L);
}

#endif // !ANIMATION_LUA


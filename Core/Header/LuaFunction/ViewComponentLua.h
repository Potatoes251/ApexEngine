#ifndef VIEW_COMPONENT_LUA
#define VIEW_COMPONENT_LUA

#include <lua.hpp>

#include "LuaManager.h"

namespace Apex::Perception { class ViewComponent; }

namespace Apex::Perception
{
	void RegisterLua(Scripting::LuaManager& lua);

	int Lua_ViewComp_SeesTag(lua_State* L);
	int Lua_ViewComp_GetClosestWithTag(lua_State* L);

	int Lua_GetViewComp(lua_State* L);
}


#endif // !VIEW_COMPONENT_LUA


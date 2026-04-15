#ifndef COMPONENT_LUA
#define COMPONENT_LUA

#include <lua.hpp>
#include "LuaManager.h"

namespace Apex { class Component; }
namespace Apex::Data { class Object; }

namespace Apex::Comp
{
	void RegisterLua(Scripting::LuaManager& lua);

	Component* GetComponent(lua_State* L, int index);

	void PushLuaObject(lua_State* L, Data::Object* obj);

	int Lua_GetOwner(lua_State* L);
}


#endif // !COMPONENT_LUA


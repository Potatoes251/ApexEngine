#ifndef COMPONENT_LUA
#define COMPONENT_LUA

#include <lua.hpp>
#include "LuaManager.h"

namespace Apex { class Component; }
namespace Apex::Data { class Object; }

namespace Apex::Comp
{
	void RegisterLua(Scripting::LuaManager& lua);

	template<typename T>
	T* GetComponent(lua_State* L, int index)
	{
		lua_getfield(L, index, "__component");
		T* comp = static_cast<T*>(lua_touserdata(L, -1));
		lua_pop(L, 1);

		if (comp)
			return comp;

		lua_getfield(L, index, "__object");
		comp = static_cast<T*>(lua_touserdata(L, -1));
		lua_pop(L, 1);
		return comp;
	}

	void PushLuaObject(lua_State* L, Data::Object* obj);

	int Lua_GetOwner(lua_State* L);
	int Lua_SetEnabled(lua_State* L);
	int Lua_AddComponent(lua_State* L);
}


#endif // !COMPONENT_LUA


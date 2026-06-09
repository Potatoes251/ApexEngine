#ifndef OBJECT_LUA
#define OBJECT_LUA

#include <lua.hpp>
#include "LuaManager.h"

namespace Apex::Data
{
	class Object;

	void RegisterLua(Scripting::LuaManager& lua);

	Object* GetObject(lua_State* L, int index);

	int Lua_GetName(lua_State* L);
	int Lua_AddTag(lua_State* L);
	int Lua_HasTag(lua_State* L);
	int Lua_GetTags(lua_State* L);
	int Lua_Destroy(lua_State* L);
	int Lua_DestroyCollectible(lua_State* L);
	int Lua_CreateObject(lua_State* L);
	int Lua_CreateCube(lua_State* L);
	int Lua_CreateSphere(lua_State* L);
}

#endif // !OBJECT_LUA
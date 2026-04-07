#include "LuaFunction/DebugLuaBindings.h"
#include "LuaFunction/Debug.h"

int Apex::Engine::Debug::Lua_ApexPrint(lua_State* L)
{
	const char* text = luaL_checkstring(L, 1);

	ApexPrint((char*)text);

	return 0;
}
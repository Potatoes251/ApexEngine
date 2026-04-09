#ifndef DEBUG_LUA_BINDINGS
#define DEBUG_LUA_BINDINGS

#include <lua.hpp>

namespace Apex::Engine::Debug
{
	int Lua_ApexPrint(lua_State* L);
}

#endif // DEBUG_LUA_BINDINGS
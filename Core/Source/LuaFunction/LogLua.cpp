#include "LuaFunction/LogLua.h"

#include "LogSystem.h"

void Apex::Log::RegisterLua(Scripting::LuaManager& lua)
{
	lua.RegisterFunction("Log", Apex::Log::Lua_Log);
}


int Apex::Log::Lua_Log(lua_State* L)
{
	const char* msg = luaL_checkstring(L, 1);

	Apex::LogSystem::Log(Debug, "Script", msg);
	return 0;
}

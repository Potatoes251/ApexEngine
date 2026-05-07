#include "LuaFunction/SceneLua.h"

#include "Application.h"

void Apex::Scene::RegisterLua(Scripting::LuaManager& lua)
{
	lua.RegisterFunction("LoadScene", Apex::Scene::Lua_Scene_LoadScene);
}

int Apex::Scene::Lua_Scene_LoadScene(lua_State* L)
{
	const char* scenePath = luaL_checkstring(L, 1);

	if (!scenePath) return 0;

	Application::Get()->SetTargetScene(scenePath);

	return 0;
}

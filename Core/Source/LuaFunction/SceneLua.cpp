#include "LuaFunction/SceneLua.h"

#include "Application.h"

void Apex::Scene::RegisterLua(Scripting::LuaManager& lua)
{
	lua.RegisterFunction("LoadScene", Apex::Scene::Lua_Scene_LoadScene);
	lua.RegisterFunction("PauseScene", Apex::Scene::Lua_Scene_PauseScene);
	lua.RegisterFunction("UnpauseScene", Apex::Scene::Lua_Scene_UnpauseScene);
}

int Apex::Scene::Lua_Scene_LoadScene(lua_State* L)
{
	const char* scenePath = luaL_checkstring(L, 1);

	if (!scenePath) return 0;

	Application::Get()->SetTargetScene(scenePath);

	return 0;
}

int Apex::Scene::Lua_Scene_PauseScene(lua_State* L)
{
	Application::Get()->Pause();

	return 0;
}

int Apex::Scene::Lua_Scene_UnpauseScene(lua_State* L)
{
	Application::Get()->Unpause();

	return 0;
}

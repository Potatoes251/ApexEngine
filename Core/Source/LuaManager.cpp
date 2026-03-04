#include "LuaManager.h"

#include "Debug.h"
#include "DebugLuaBindings.h"
#include "TransformFunc.h"
#include "HierarchyFunc.h"

#include <iostream>

Apex::Scripting::LuaManager::LuaManager()
{
	luaState = luaL_newstate();
	luaL_openlibs(luaState);

    lua_getglobal(luaState, "package");
    lua_getfield(luaState, -1, "path");

    std::string currentPath = lua_tostring(luaState, -1);
    lua_pop(luaState, 1);

    currentPath += ";../../Assets/Scripts/?.lua";

    lua_pushstring(luaState, currentPath.c_str());
    lua_setfield(luaState, -2, "path");
    lua_pop(luaState, 1);

	RegisterFunctions();
}

Apex::Scripting::LuaManager::~LuaManager()
{
	for (int ref : m_scriptRefs)
		luaL_unref(luaState, LUA_REGISTRYINDEX, ref);
	lua_close(luaState);
}

int Apex::Scripting::LuaManager::LoadScript(const std::string& path)
{
    if (luaL_dofile(luaState, path.c_str()) != LUA_OK)
    {
        const char* msg = lua_tostring(luaState, -1);
        std::cerr << "Lua error: " << (msg ? msg : "unknown") << std::endl;
        lua_pop(luaState, 1);
        return LUA_NOREF;
    }

    if (!lua_istable(luaState, -1))
    {
        std::cerr << "Lua error: script did not return a table!" << std::endl;
        lua_pop(luaState, 1);
        return LUA_NOREF;
    }

    int ref = luaL_ref(luaState, LUA_REGISTRYINDEX);
    m_scriptRefs.push_back(ref);
    return ref;
}

void Apex::Scripting::LuaManager::RegisterFunction(const char* name, lua_CFunction func)
{
	lua_register(luaState, name, func);
}

void Apex::Scripting::LuaManager::RegisterFunctions()
{
	RegisterFunction("Print", Apex::Engine::Debug::Lua_ApexPrint);
    RegisterFunction("SetPosition_Internal", Apex::Transform::Lua_SetPosition);
    RegisterFunction("AddChild_Internal", Apex::Hierarchy::Lua_AddChild);
    RegisterFunction("RemoveChild_Internal", Apex::Hierarchy::Lua_RemoveChild);
    RegisterFunction("Reparent_Internal", Apex::Hierarchy::Lua_SetParent);
}

int Apex::Scripting::LuaManager::CreateScriptInstance(int scriptRef)
{
    lua_rawgeti(luaState, LUA_REGISTRYINDEX, scriptRef);
    lua_getfield(luaState, -1, "New");
    if (!lua_isfunction(luaState, -1))
    {
        lua_pop(luaState, 1);
        return LUA_NOREF;
    }

    lua_pushvalue(luaState, -2);
    if (lua_pcall(luaState, 1, 1, 0) != LUA_OK)
    {
        std::cerr << "Lua error: " << lua_tostring(luaState, -1) << std::endl;
        lua_pop(luaState, 1);
        return LUA_NOREF;
    }

    int instanceRef = luaL_ref(luaState, LUA_REGISTRYINDEX);
    lua_pop(luaState, 1);
    return instanceRef;
}

bool Apex::Scripting::LuaManager::CallFunction(int instanceRef, const std::string& funcName, float dt)
{
    if (instanceRef == LUA_NOREF) return false;

    lua_rawgeti(luaState, LUA_REGISTRYINDEX, instanceRef); // push instance
    if (!lua_istable(luaState, -1))
    {
        lua_pop(luaState, 1);
        return false;
    }

    lua_getfield(luaState, -1, funcName.c_str()); // push function
    if (!lua_isfunction(luaState, -1))
    {
        lua_pop(luaState, 1);
        lua_pop(luaState, 1);
        return false;
    }

    lua_pushvalue(luaState, -2); // push self
    int args = 1;

    lua_pushnumber(luaState, dt);
    args++;

    if (lua_pcall(luaState, args, 0, 0) != LUA_OK)
    {
        std::cerr << "Lua error: " << lua_tostring(luaState, -1) << std::endl;
        lua_pop(luaState, 1);
        lua_pop(luaState, 1);
        return false;
    }

    lua_pop(luaState, 1); // pop instance
    return true;
}
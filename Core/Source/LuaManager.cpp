#include "LuaManager.h"

#include "Log.h"

#include "LuaFunction/LuaFunctions.h"

#include <iostream>

Apex::Scripting::LuaManager::LuaManager()
{
	m_luaState = luaL_newstate();
	luaL_openlibs(m_luaState);

    lua_getglobal(m_luaState, "package");
    lua_getfield(m_luaState, -1, "path");

    std::string currentPath = lua_tostring(m_luaState, -1);
    lua_pop(m_luaState, 1);

    currentPath += "Assets/Scripts/?.lua";

    lua_pushstring(m_luaState, currentPath.c_str());
    lua_setfield(m_luaState, -2, "path");
    lua_pop(m_luaState, 1);

	RegisterFunctions();
}

Apex::Scripting::LuaManager::~LuaManager()
{
	for (int ref : m_scriptRefs)
		luaL_unref(m_luaState, LUA_REGISTRYINDEX, ref);
	lua_close(m_luaState);
}

int Apex::Scripting::LuaManager::LoadScript(const std::string& path)
{
    if (luaL_dofile(m_luaState, path.c_str()) != LUA_OK)
    {
        const char* msg = lua_tostring(m_luaState, -1);
        LOG_ERROR("Lua error: {}", (msg ? msg : "unknown"));
        lua_pop(m_luaState, 1);
        return LUA_NOREF;
    }

    if (!lua_istable(m_luaState, -1))
    {
        LOG_ERROR("Lua error: script did not return a table!");
        lua_pop(m_luaState, 1);
        return LUA_NOREF;
    }

    int ref = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
    m_scriptRefs.push_back(ref);
    return ref;
}

void Apex::Scripting::LuaManager::RegisterFunction(const char* name, lua_CFunction func)
{
	lua_register(m_luaState, name, func);
}

void Apex::Scripting::LuaManager::RegisterFunctions()
{
    Apex::Log::RegisterLua(*this);
    Apex::Anim::RegisterLua(*this);
    Apex::Comp::RegisterLua(*this);
    Apex::Data::RegisterLua(*this);
    Apex::Mezh::RegisterLua(*this);
    Apex::Audio::RegisterLua(*this);
    Apex::Input::RegisterLua(*this);
    Apex::Scene::RegisterLua(*this);
    Apex::Camera::RegisterLua(*this);
    Apex::Vector3::RegisterLua(*this);
    Apex::Hierarchy::RegisterLua(*this);
    Apex::Transform::RegisterLua(*this);
    Apex::Controller::RegisterLua(*this);
    Apex::Perception::RegisterLua(*this);
	Apex::Gameplay::RegisterLua(*this);
    Apex::Mouse::RegisterLua(*this);
}

int Apex::Scripting::LuaManager::CreateScriptInstance(int scriptRef)
{
    lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, scriptRef);
    lua_getfield(m_luaState, -1, "New");
    if (!lua_isfunction(m_luaState, -1))
    {
        lua_pop(m_luaState, 1);
        return LUA_NOREF;
    }

    lua_pushvalue(m_luaState, -2);
    if (lua_pcall(m_luaState, 1, 1, 0) != LUA_OK)
    {
        LOG_ERROR("Lua error: {}", lua_tostring(m_luaState, -1));
        lua_pop(m_luaState, 1);
        return LUA_NOREF;
    }

    int instanceRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
    lua_pop(m_luaState, 1);
    return instanceRef;
}

bool Apex::Scripting::LuaManager::CallFunction(int instanceRef, const std::string& funcName)
{
    if (instanceRef == LUA_NOREF) return false;

    lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, instanceRef); // push instance
    if (!lua_istable(m_luaState, -1))
    {
        lua_pop(m_luaState, 1);
        return false;
    }

    lua_getfield(m_luaState, -1, funcName.c_str()); // push function
    if (!lua_isfunction(m_luaState, -1))
    {
        lua_pop(m_luaState, 1);
        lua_pop(m_luaState, 1);
        return false;
    }

    lua_pushvalue(m_luaState, -2); // push self
    int args = 1;

    if (lua_pcall(m_luaState, args, 0, 0) != LUA_OK)
    {
        LOG_ERROR("Lua error: {}", lua_tostring(m_luaState, -1));
        lua_pop(m_luaState, 1);
        lua_pop(m_luaState, 1);
        return false;
    }

    lua_pop(m_luaState, 1); // pop instance
    return true;
}

bool Apex::Scripting::LuaManager::CallFunction(int instanceRef, const std::string& funcName, float dt)
{
    if (instanceRef == LUA_NOREF) return false;

    lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, instanceRef); // push instance
    if (!lua_istable(m_luaState, -1))
    {
        lua_pop(m_luaState, 1);
        return false;
    }

    lua_getfield(m_luaState, -1, funcName.c_str()); // push function
    if (!lua_isfunction(m_luaState, -1))
    {
        lua_pop(m_luaState, 1);
        lua_pop(m_luaState, 1);
        return false;
    }

    lua_pushvalue(m_luaState, -2); // push self
    int args = 1;

    lua_pushnumber(m_luaState, dt);
    args++;

    if (lua_pcall(m_luaState, args, 0, 0) != LUA_OK)
    {
        LOG_ERROR("Lua error: {}", lua_tostring(m_luaState, -1));
        lua_pop(m_luaState, 1);
        lua_pop(m_luaState, 1);
        return false;
    }

    lua_pop(m_luaState, 1); // pop instance
    return true;
}

bool Apex::Scripting::LuaManager::CallFunction(int instanceRef, const std::string& funcName, Apex::Physic::Collider* collider)
{
    if (instanceRef == LUA_NOREF) return false;

    lua_getglobal(m_luaState, "require");
    lua_pushstring(m_luaState, "Collider");
    lua_pcall(m_luaState, 1, 1, 0);

    lua_getfield(m_luaState, -1, "New");
    lua_pushvalue(m_luaState, -2);
    lua_pushlightuserdata(m_luaState, collider);

    if (lua_pcall(m_luaState, 2, 1, 0) != LUA_OK)
        return false;

    int colliderRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);

    lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, instanceRef);

    lua_getfield(m_luaState, -1, funcName.c_str());
    if (!lua_isfunction(m_luaState, -1))
    {
        lua_pop(m_luaState, 2);
        return false;
    }

    lua_pushvalue(m_luaState, -2); // self
    lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, colliderRef);

    if (lua_pcall(m_luaState, 2, 0, 0) != LUA_OK)
    {
        LOG_ERROR("Lua error: {}", lua_tostring(m_luaState, -1));
        lua_pop(m_luaState, 1);
        return false;
    }

    lua_pop(m_luaState, 1); // instance
    return true;
}

void Apex::Scripting::LuaManager::RegisterHudBindings(Apex::UserInterface::UIManager* ui)
{
    Apex::HudBindings::RegisterHudFunctions(m_luaState, ui);
}

void Apex::Scripting::LuaManager::RegisterScriptCast(Application* app)
{
    Apex::ScriptCast::Register(m_luaState, app);
}

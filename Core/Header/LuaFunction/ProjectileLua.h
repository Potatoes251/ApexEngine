#ifndef PROJECTILE_LUA
#define PROJECTILE_LUA

#include <lua.hpp>
#include "LuaManager.h"

namespace Apex::Gameplay
{
    void RegisterLua(Scripting::LuaManager& lua);

    int Lua_ProjectileSetup(lua_State* L);
    int Lua_GetProjectileComponent(lua_State* L);
}

#endif
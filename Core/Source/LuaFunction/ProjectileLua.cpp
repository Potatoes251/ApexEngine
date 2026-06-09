#include "LuaFunction/ProjectileLua.h"
#include "LuaFunction/ComponentLua.h"
#include "LuaFunction/Vector3Lua.h"

#include "ProjectileComponent.h"
#include "Object.h"

void Apex::Gameplay::RegisterLua(Scripting::LuaManager& lua)
{
	lua.RegisterFunction("GetProjectileComponent_Internal", Apex::Gameplay::Lua_GetProjectileComponent);
	lua.RegisterFunction("ProjectileSetup_Internal", Apex::Gameplay::Lua_ProjectileSetup);
}

int Apex::Gameplay::Lua_ProjectileSetup(lua_State* L)
{
    ProjectileComponent* projectile = Comp::GetComponent<ProjectileComponent>(L, 1);
    if (!projectile) return 0;

    LibMath::Vector3* dirPtr = static_cast<LibMath::Vector3*>(lua_touserdata(L, 2));
    LibMath::Vector3 direction = dirPtr ? *dirPtr : LibMath::Vector3{ 0.f, 0.f, 1.f };

    float speed = static_cast<float>(lua_tonumber(L, 3));
    float lifeSpan = static_cast<float>(lua_tonumber(L, 4));

    projectile->Setup(direction, speed, lifeSpan);
    return 0;
}

int Apex::Gameplay::Lua_GetProjectileComponent(lua_State* L)
{
    Object* obj = static_cast<Object*>(lua_touserdata(L, 1));
    if (!obj) return 0;

    ProjectileComponent* projectile = obj->GetComponent<ProjectileComponent>();
    if (!projectile) return 0;

    lua_pushlightuserdata(L, projectile);
    return 1;
}

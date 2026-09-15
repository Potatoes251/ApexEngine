#include "LuaFunction/ComponentLua.h"
#include "LuaFunction/ObjectLua.h"

#include "Component.h"
#include "Object.h"

#include "ProjectileComponent.h"
#include "AudioComponent.h"
#include "RigidBody.h"
#include "Colliders.h"
#include "MeshRenderer.h"

#include "Application.h"
#include "Scene.h"

using namespace Apex;
using namespace Apex::Data;

void Apex::Comp::RegisterLua(Scripting::LuaManager& lua)
{
    lua.RegisterFunction("GetOwner_Internal", Apex::Comp::Lua_GetOwner);
    lua.RegisterFunction("SetEnabled_Internal", Apex::Comp::Lua_SetEnabled);
	lua.RegisterFunction("AddComponent_Internal", Apex::Comp::Lua_AddComponent);
}

void Apex::Comp::PushLuaObject(lua_State* L, Object* obj)
{
    if (!obj)
    {
        lua_pushnil(L);
        return;
    }

    lua_getglobal(L, "require");
    lua_pushstring(L, "Object");
    lua_pcall(L, 1, 1, 0);

    // call Object:New()
    lua_getfield(L, -1, "New");
    lua_pushvalue(L, -2);
    lua_pcall(L, 1, 1, 0);

    // stack now: instance
    lua_pushlightuserdata(L, obj);
    lua_setfield(L, -2, "__object");

    // remove Object table
    lua_remove(L, -2);
}

int Apex::Comp::Lua_GetOwner(lua_State* L)
{
    Component* comp = GetComponent<Component>(L, 1);
    if (!comp) return 0;

    Object* owner = comp->GetOwner();
    PushLuaObject(L, owner);
    return 1;
}

int Apex::Comp::Lua_SetEnabled(lua_State* L)
{
    Component* comp = GetComponent<Component>(L, 1);
    if (!comp) return 0;

    bool enabled = lua_toboolean(L, 2);

    comp->SetEnabled(enabled);
    return 0;
}

int Apex::Comp::Lua_AddComponent(lua_State* L)
{
    Apex::Data::Object* obj = Apex::Data::GetObject(L, 1);
    const char* compType = luaL_checkstring(L, 2);

    Application* app = Application::Get();
    Apex::Physic::PhysicSystem* physicSystem = (app && app->GetScene()) ? app->GetScene()->GetPhysic() : nullptr;
	Apex::Resources::ResourceManager* resourceManager = app ? app->GetResourceManager() : nullptr;

    if (!obj || !compType) return 0;

    if (strcmp(compType, "MeshRendererComponent") == 0)
    {
        obj->AddComponent<Apex::Rendering::MeshRenderer>(resourceManager);
    }
    else if (strcmp(compType, "ProjectileComponent") == 0)
    {
        obj->AddComponent<Apex::Gameplay::ProjectileComponent>(LibMath::Vector3(0.f, 0.f, 1.f), 20.f, 3.f);
    }
    else if (strcmp(compType, "RigidBodyComponent") == 0)
    {
        obj->AddComponent<Apex::Physic::RigidBodyComponent>(1.f, Apex::Physic::BodyType::Kinematic);
    }
    else if (strcmp(compType, "BoxColliderComponent") == 0)
    {
        auto& comp = obj->AddComponent<Apex::Physic::BoxCollider>(LibMath::Vector3(1.0f, 1.0f, 1.0f), false);
        physicSystem->CreateActor(comp, *obj->GetComponent<Apex::Physic::RigidBodyComponent>());
    }
    else if (strcmp(compType, "CapsuleColliderComponent") == 0)
    {
        auto& comp = obj->AddComponent<Apex::Physic::CapsuleCollider>(0.5f, 0.2f, false);
		physicSystem->CreateActor(comp, *obj->GetComponent<Apex::Physic::RigidBodyComponent>());
    }
    else if (strcmp(compType, "MeshColliderComponent") == 0)
    {
        Apex::Rendering::MeshRenderer* meshRenderer = obj->GetComponent<Apex::Rendering::MeshRenderer>();
		if (meshRenderer)
		{
            auto& comp = obj->AddComponent<Apex::Physic::MeshCollider>(meshRenderer->GetModel(), Apex::Physic::MeshColliderType::Convex, false);
			physicSystem->CreateActor(comp, *obj->GetComponent<Apex::Physic::RigidBodyComponent>());
		}
    }
	else if (strcmp(compType, "AudioComponent") == 0)
	{
		obj->AddComponent<Apex::Audio::AudioComponent>(nullptr);
	}

    return 0;
}

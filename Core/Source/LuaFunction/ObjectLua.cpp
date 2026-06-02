#include "LuaFunction/ObjectLua.h"
#include "LuaFunction/ComponentLua.h"

#include "Object.h"

#include "Application.h"


using namespace Apex::Data;


void Apex::Data::RegisterLua(Scripting::LuaManager& lua)
{
    lua.RegisterFunction("AddTag_Internal", Apex::Data::Lua_AddTag);
    lua.RegisterFunction("HasTag_Internal", Apex::Data::Lua_HasTag);
    lua.RegisterFunction("GetTags_Internal", Apex::Data::Lua_GetTags);
    lua.RegisterFunction("Destroy_Internal", Apex::Data::Lua_Destroy);
	lua.RegisterFunction("GetName_Internal", Apex::Data::Lua_GetName);
    lua.RegisterFunction("DestroyCollectible_Internal", Apex::Data::Lua_DestroyCollectible);
	lua.RegisterFunction("CreateObject_Internal", Apex::Data::Lua_CreateObject);
    lua.RegisterFunction("CreateCube_Internal", Apex::Data::Lua_CreateCube);
	lua.RegisterFunction("CreateSphere_Internal", Apex::Data::Lua_CreateSphere);
}


Object* Apex::Data::GetObject(lua_State* L, int index)
{
    lua_getfield(L, index, "__object");
    auto* obj = static_cast<Apex::Data::Object*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return obj;
}

int Apex::Data::Lua_GetName(lua_State* L)
{
	Object* obj = GetObject(L, 1);
	if (!obj) return 0;

	lua_pushstring(L, obj->GetName().c_str());
	return 1;
}

int Apex::Data::Lua_AddTag(lua_State* L)
{
    Object* obj = GetObject(L, 1);
    
    if (!obj) return 0;

    const char* tag = luaL_checkstring(L, 2);

    obj->AddTag(tag);

    return 0;
}

int Apex::Data::Lua_HasTag(lua_State* L)
{
    Object* obj = GetObject(L, 1);

    if (!obj) return 0;

    const char* tag = luaL_checkstring(L, 2);

    lua_pushboolean(L, obj->HasTag(tag));

    return 1;
}

int Apex::Data::Lua_GetTags(lua_State* L)
{
    Object* obj = GetObject(L, 1);

    if (!obj) return 0;

    std::unordered_set<std::string> const& tags = obj->GetTags();

    lua_newtable(L); // create table

    int index = 1;
    for (std::string const& tag : tags)
    {
        lua_pushinteger(L, index++);     // key
        lua_pushstring(L, tag.c_str());  // value
        lua_settable(L, -3);             // table[key] = value
    }

    return 1;
}

int Apex::Data::Lua_Destroy(lua_State* L)
{    
    Object* obj = GetObject(L, 1);

    if (!obj) return 0;

    obj->Destroy();

    return 0;
}

int Apex::Data::Lua_DestroyCollectible(lua_State* L)
{
    Object* obj = GetObject(L, 1);

    if (!obj)
        return 0;

    Apex::Application::Get()->AddCollectedObject(obj->GetId());

    obj->Destroy();

    return 0;
}

int Apex::Data::Lua_CreateObject(lua_State* L)
{
    const char* name = lua_gettop(L) >= 1 ? luaL_checkstring(L, 1) : nullptr;

    Application* app = Application::Get();
    if (!app || !app->GetScene())
    {
        lua_pushnil(L);
        return 1;
    }

    Object* newObj = app->GetScene()->CreateObjectRunTime();
    if (!newObj)
    {
        lua_pushnil(L);
        return 1;
    }

    if (name)
    {
        newObj->SetName(name); 
    }

    Apex::Comp::PushLuaObject(L, newObj);

    return 1; 
}

int Apex::Data::Lua_CreateCube(lua_State* L)
{
    const char* name = lua_gettop(L) >= 1 ? luaL_checkstring(L, 1) : nullptr;

    Application* app = Application::Get();
    if (!app || !app->GetScene())
    {
        lua_pushnil(L);
        return 1;
    }

    Object* newObj = app->GetScene()->CreateObjectRunTime();
    if (!newObj)
    {
        lua_pushnil(L);
        return 1;
    }

    Apex::Resources::ResourceManager* resourceManager = app ? app->GetResourceManager() : nullptr;

    auto& comp = newObj->AddComponent<Apex::Rendering::MeshRenderer>();

    auto mesh = resourceManager->CreateAsync<Model>("ApexAssets/Meshes/Cube/Cube.mesh");
    comp.SetModel(mesh);
    auto mat = resourceManager->CreateAsync<Apex::Rendering::Material>("Assets/Material/Default.mat");
    comp.SetMaterials({ mat });

    if (name)
    {
        newObj->SetName(name);
    }

    Apex::Comp::PushLuaObject(L, newObj);

    return 1;
}

int Apex::Data::Lua_CreateSphere(lua_State* L)
{
	const char* name = lua_gettop(L) >= 1 ? luaL_checkstring(L, 1) : nullptr;

	Application* app = Application::Get();
	if (!app || !app->GetScene())
	{
		lua_pushnil(L);
		return 1;
	}

	Object* newObj = app->GetScene()->CreateObjectRunTime();
	if (!newObj)
	{
		lua_pushnil(L);
		return 1;
	}

	Apex::Resources::ResourceManager* resourceManager = app ? app->GetResourceManager() : nullptr;

	auto& comp = newObj->AddComponent<Apex::Rendering::MeshRenderer>();
	auto mesh = resourceManager->CreateAsync<Model>("ApexAssets/Meshes/Sphere/Sphere.mesh");
	comp.SetModel(mesh);
	auto mat = resourceManager->CreateAsync<Apex::Rendering::Material>("Assets/Material/Default.mat");
    comp.SetMaterials({ mat });

	if (name)
	{
		newObj->SetName(name);
	}

	Apex::Comp::PushLuaObject(L, newObj);

	return 1;
}

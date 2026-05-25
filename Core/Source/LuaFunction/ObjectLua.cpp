#include "LuaFunction/ObjectLua.h"

#include "Object.h"

using namespace Apex::Data;


void Apex::Data::RegisterLua(Scripting::LuaManager& lua)
{
    lua.RegisterFunction("AddTag_Internal", Apex::Data::Lua_AddTag);
    lua.RegisterFunction("HasTag_Internal", Apex::Data::Lua_HasTag);
    lua.RegisterFunction("GetTags_Internal", Apex::Data::Lua_GetTags);
    lua.RegisterFunction("Destroy_Internal", Apex::Data::Lua_Destroy);
	lua.RegisterFunction("GetName_Internal", Apex::Data::Lua_GetName);
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

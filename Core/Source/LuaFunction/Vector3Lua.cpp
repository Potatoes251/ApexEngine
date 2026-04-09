#include "LuaFunction/Vector3Lua.h"

using namespace Apex::Vector3;

void Apex::Vector3::RegisterLua(Scripting::LuaManager& lua)
{
    lua_State* L = lua.GetState();

    luaL_newmetatable(L, "Vector3");

    lua_pushcfunction(L, Lua_Vector3_Add);
    lua_setfield(L, -2, "__add");

    lua_pushcfunction(L, Lua_Vector3_Mul);
    lua_setfield(L, -2, "__mul");

    lua_pushcfunction(L, Lua_Vector3_Index);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, Lua_Vector3_NewIndex);
    lua_setfield(L, -2, "__newindex");

    // methods
    lua_pushcfunction(L, Lua_Vector3_Normalized);
    lua_setfield(L, -2, "Normalized");

    lua_pop(L, 1);
}

int Apex::Vector3::Lua_Vector3_Normalized(lua_State* L)
{
    auto* v = CheckVector3(L, 1);

    PushVector3(L, v->m_value.normalized());
    return 1;
}

int Apex::Vector3::Lua_Vector3_Add(lua_State* L)
{
    LuaVector3* a = CheckVector3(L, 1);
    LuaVector3* b = CheckVector3(L, 2);

    PushVector3(L, a->m_value + b->m_value);
    return 1;
}

int Apex::Vector3::Lua_Vector3_Mul(lua_State* L)
{
    LuaVector3* v = CheckVector3(L, 1);
    float s = (float)luaL_checknumber(L, 2);

    PushVector3(L, v->m_value * s);

    return 1;
}

int Apex::Vector3::Lua_Vector3_Index(lua_State* L)
{
    auto* v = CheckVector3(L, 1);
    const char* key = luaL_checkstring(L, 2);

    if (strcmp(key, "x") == 0) 
    { 
        lua_pushnumber(L, v->m_value[0]); 
        return 1; 
    }
    if (strcmp(key, "y") == 0) 
    { 
        lua_pushnumber(L, v->m_value[1]); 
        return 1; 
    }
    if (strcmp(key, "z") == 0)
    { 
        lua_pushnumber(L, v->m_value[2]); 
        return 1; 
    }

    // fallback to methods
    luaL_getmetatable(L, "Vector3");
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    return 1;
}

int Apex::Vector3::Lua_Vector3_NewIndex(lua_State* L)
{
    auto* v = CheckVector3(L, 1);
    const char* key = luaL_checkstring(L, 2);
    float value = (float)luaL_checknumber(L, 3);

    if (strcmp(key, "x") == 0) v->m_value[0] = value;
    if (strcmp(key, "y") == 0) v->m_value[1] = value;
    if (strcmp(key, "z") == 0) v->m_value[2] = value;

    return 0;
}
#ifndef VECTOR3_LUA
#define VECTOR3_LUA

#include <lua.hpp>

#include "LibMath/Vector/Vector3.h"
#include "LuaManager.h"

namespace Apex::Vector3
{
    void RegisterLua(Scripting::LuaManager& lua);

    struct LuaVector3
    {
        LibMath::Vector3 m_value;
    };

    static LuaVector3* CheckVector3(lua_State* L, int index)
    {
        return (LuaVector3*)luaL_checkudata(L, index, "Vector3");
    }

    static LuaVector3* PushVector3(lua_State* L, const LibMath::Vector3& v)
    {
        LuaVector3* out = (LuaVector3*)lua_newuserdata(L, sizeof(LuaVector3));
        out->m_value = v;

        luaL_getmetatable(L, "Vector3");
        lua_setmetatable(L, -2);

        return out;
    }

    int Lua_Vector3_New(lua_State* L);
    int Lua_Vector3_GarbageCollect(lua_State* L);
    int Lua_Vector3_Normalized(lua_State* L);
    int Lua_Vector3_Add(lua_State* L);
    int Lua_Vector3_Mul(lua_State* L);
    int Lua_Vector3_Index(lua_State* L);
    int Lua_Vector3_NewIndex(lua_State* L);
}


#endif // !VECTOR3_LUA


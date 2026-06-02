#include "LuaFunction/TransformFunc.h"
#include "Object.h"
#include "Log.h"

namespace Apex::Transform
{
    void RegisterLua(Scripting::LuaManager& lua)
    {
		lua.RegisterFunction("GetForward_Internal", Lua_GetForward);
        lua.RegisterFunction("GetPosition_Internal", Lua_GetPosition);
        lua.RegisterFunction("GetRotation_Internal", Lua_GetRotation);
        lua.RegisterFunction("GetScale_Internal", Lua_GetScale);
        lua.RegisterFunction("SetRotation_Internal", Lua_SetRotation);
        lua.RegisterFunction("SetPosition_Internal", Lua_SetPosition);
        lua.RegisterFunction("SetScale_Internal", Lua_SetScale);
    }

	int Lua_GetForward(lua_State* L)
	{
		lua_getfield(L, 1, "__object");
		auto* obj = static_cast<Apex::Data::Object*>(lua_touserdata(L, -1));
		lua_pop(L, 1);

		if (!obj) return 0;

		auto forward = obj->GetLocalTransform().getForward();
		lua_pushnumber(L, forward[0]);
		lua_pushnumber(L, forward[1]);
		lua_pushnumber(L, forward[2]);
		return 3; // Returning 3 values: x, y, z
	}

    int Lua_GetPosition(lua_State* L)
    {
        lua_getfield(L, 1, "__object");
        auto* obj = static_cast<Apex::Data::Object*>(lua_touserdata(L, -1));
        lua_pop(L, 1);

        if (!obj) return 0;

        auto pos = obj->GetLocalTransform().getPosition();
        lua_pushnumber(L, pos[0]);
        lua_pushnumber(L, pos[1]);
        lua_pushnumber(L, pos[2]);

        return 3; // Returning 3 values: x, y, z
    }

    int Lua_GetRotation(lua_State* L)
    {
        lua_getfield(L, 1, "__object");
        auto* obj = static_cast<Apex::Data::Object*>(lua_touserdata(L, -1));
        lua_pop(L, 1);

        if (!obj) return 0;

        auto rot = obj->GetLocalTransform().getRotation();
        lua_pushnumber(L, rot[0]);
        lua_pushnumber(L, rot[1]);
        lua_pushnumber(L, rot[2]);
        lua_pushnumber(L, rot[3]);

        return 4; // Returning 4 values: x, y, z, w
    }

    int Lua_GetScale(lua_State* L)
    {
        lua_getfield(L, 1, "__object");
        auto* obj = static_cast<Apex::Data::Object*>(lua_touserdata(L, -1));
        lua_pop(L, 1);

        if (!obj) return 0;

        auto scale = obj->GetLocalTransform().getScale();
        lua_pushnumber(L, scale[0]);
        lua_pushnumber(L, scale[1]);
        lua_pushnumber(L, scale[2]);

        return 3; // Returning 3 values: x, y, z
    }

    int Lua_SetPosition(lua_State* L)
    {
        lua_getfield(L, 1, "__object");
        auto* obj = static_cast<Apex::Data::Object*>(lua_touserdata(L, -1));
        lua_pop(L, 1);

        if (!obj)
            return 0;

        float x = (float)luaL_checknumber(L, 2);
        float y = (float)luaL_checknumber(L, 3);
        float z = (float)luaL_checknumber(L, 4);

        LibMath::Transform trans;
        trans = obj->GetLocalTransform();
        trans.setPosition({ x, y, z });

        obj->SetLocalTransform(trans);

        return 0;
    }

    int Lua_SetRotation(lua_State* L)
    {
        lua_getfield(L, 1, "__object");
        auto* obj = static_cast<Apex::Data::Object*>(lua_touserdata(L, -1));
        lua_pop(L, 1);

        if (!obj)
            return 0;

        float x = (float)luaL_checknumber(L, 2);
        float y = (float)luaL_checknumber(L, 3);
        float z = (float)luaL_checknumber(L, 4);
		float w = (float)luaL_checknumber(L, 5);

        LibMath::Transform trans;
        trans = obj->GetLocalTransform();
        trans.setRotation({ x, y, z, w});

        obj->SetLocalTransform(trans);

        return 0;
        
    }

    int Lua_SetScale(lua_State* L)
    {
        lua_getfield(L, 1, "__object");
        auto* obj = static_cast<Apex::Data::Object*>(lua_touserdata(L, -1));
        lua_pop(L, 1);

        if (!obj)
            return 0;

        float x = (float)luaL_checknumber(L, 2);
        float y = (float)luaL_checknumber(L, 3);
        float z = (float)luaL_checknumber(L, 4);

        LibMath::Transform trans;
        trans = obj->GetLocalTransform();
        trans.setScale({ x, y, z });

        obj->SetLocalTransform(trans);

        return 0;
    }
}
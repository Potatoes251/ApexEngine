#include "LuaFunction/TransformFunc.h"
#include "Object.h"

namespace Apex::Transform
{
    void RegisterLua(Scripting::LuaManager& lua)
    {
        lua.RegisterFunction("SetPositiob_Internal", Lua_SetPosition);
        lua.RegisterFunction("SetScale_Internal", Lua_SetScale);
    }

    int Lua_SetPosition(lua_State* L)
    {
        std::cout << "SetPosition called\n";

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

    int Lua_SetScale(lua_State* L)
    {
        std::cout << "SetPosition called\n";

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
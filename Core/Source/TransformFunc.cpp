#include "TransformFunc.h"
#include "Object.h"

namespace Apex::Transform
{
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

        LibMath::Transform t;
        // récupère l'ancien transform
        // modifie sa position
        t = obj->GetLocalTransform();
        t.setPosition({ x, y, z });

        obj->SetLocalTransform(t);

        return 0;
    }
}
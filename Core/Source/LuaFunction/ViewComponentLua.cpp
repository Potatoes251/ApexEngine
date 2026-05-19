#include "LuaFunction/ViewComponentLua.h"
#include "LuaFunction/ObjectLua.h"
#include "LuaFunction/ComponentLua.h"

#include "Object.h"

#include "ViewComponent.h"

using namespace Apex::Data;
using namespace Apex::Perception;

void Apex::Perception::RegisterLua(Scripting::LuaManager& lua)
{
    lua.RegisterFunction("GetViewComp_Internal", Apex::Perception::Lua_GetViewComp);
    lua.RegisterFunction("ViewComp_SeesTag_Internal", Apex::Perception::Lua_ViewComp_SeesTag);
    lua.RegisterFunction("ViewComp_GetClosestWithTag_Internal", Apex::Perception::Lua_ViewComp_GetClosestWithTag);
}

int Apex::Perception::Lua_ViewComp_SeesTag(lua_State* L)
{
    ViewComponent* comp = Comp::GetComponent<ViewComponent>(L, 1);
    if (!comp)
        return 0;

    const char* tag = luaL_checkstring(L, 2);

    if (!tag)
        return 0;

    lua_pushboolean(L, comp->SeesTag(tag));

    return 1;
}

int Apex::Perception::Lua_ViewComp_GetClosestWithTag(lua_State* L)
{
    const char* tag = luaL_checkstring(L, 2);
    if (!tag)
        return 0;

    ViewComponent* comp = Comp::GetComponent<ViewComponent>(L, 1);
    if (!comp)
        return 0;

    Object* seenObj = comp->GetClosestWithTag(tag);
    if (!seenObj)
        return 0;

    // push object as userdata
    lua_pushlightuserdata(L, seenObj);

    return 1;
}

int Apex::Perception::Lua_GetViewComp(lua_State* L)
{
    Object* obj = GetObject(L, 1);
    if (!obj)
        return 0;

    ViewComponent* comp = obj->GetComponent<ViewComponent>();
    if (!comp)
        return 0;

    // push component as userdata
    lua_pushlightuserdata(L, comp);

    return 1;
}

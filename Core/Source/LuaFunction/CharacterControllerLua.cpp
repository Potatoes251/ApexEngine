#include "LuaFunction/CharacterControllerLua.h"
#include "LuaFunction/HierarchyFunc.h"
#include "LuaFunction/Vector3Lua.h"

#include "CharacterController.h"

#include "Log.h"

using namespace Apex::Data;
using namespace Apex::Vector3;
using namespace Apex::Hierarchy;
using namespace Apex::Controller;

void Apex::Controller::RegisterLua(Scripting::LuaManager& lua)
{
    lua.RegisterFunction("GetCharacterController_Internal", Apex::Controller::Lua_GetCharacterController);
    lua.RegisterFunction("CC_Move_Internal", Apex::Controller::Lua_CC_Move);
    lua.RegisterFunction("CC_Jump_Internal", Apex::Controller::Lua_CC_Jump);
    lua.RegisterFunction("CC_GetVelocity_Internal", Apex::Controller::Lua_CC_GetVelocity);
    lua.RegisterFunction("CC_IsGrounded_Internal", Apex::Controller::Lua_CC_IsGrounded);
    lua.RegisterFunction("CC_IsInWater_Internal", Apex::Controller::Lua_CC_IsInWater);
}

CharacterController* Apex::Controller::GetCC(lua_State* L, int index)
{
    lua_getfield(L, index, "__object");
    CharacterController* cc = static_cast<CharacterController*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return cc;
}

int Apex::Controller::Lua_CC_Move(lua_State* L)
{
    CharacterController* cc = GetCC(L, 1);

    if (!cc)
        return 0;

    LuaVector3* dir = CheckVector3(L, 2);

    cc->Move(dir->m_value);

    return 0;
}

int Apex::Controller::Lua_CC_Jump(lua_State* L)
{
    CharacterController* cc = GetCC(L, 1);

    if (!cc)
        return 0;

    cc->Jump();

    return 0;
}

int Apex::Controller::Lua_CC_GetVelocity(lua_State* L)
{
    CharacterController* cc = GetCC(L, 1);

    if (!cc) return 0;

    lua_pushnumber(L, cc->GetHorizontalVelocity());

    return 1;
}

int Apex::Controller::Lua_CC_IsGrounded(lua_State* L)
{
    CharacterController* cc = GetCC(L, 1);

    if (!cc) return 0;

    lua_pushboolean(L, cc->IsGrounded());

    return 1;
}

int Apex::Controller::Lua_CC_IsInWater(lua_State* L)
{
    CharacterController* cc = GetCC(L, 1);

    if (!cc) return 0;

    lua_pushboolean(L, cc->IsInWater());

    return 1;
}

int Apex::Controller::Lua_GetCharacterController(lua_State* L)
{
    Object* obj = GetObject(L, 1);
    if (!obj)
        return 0;

    CharacterController* cc = obj->GetComponent<CharacterController>();
    if (!cc)
        return 0;

    // push component as userdata
    lua_pushlightuserdata(L, cc);

    return 1;
}

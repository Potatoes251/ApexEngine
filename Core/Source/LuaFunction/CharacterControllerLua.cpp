#include "LuaFunction/CharacterControllerLua.h"
#include "LuaFunction/HierarchyFunc.h"
#include "LuaFunction/Vector3Lua.h"
#include "LuaFunction/ComponentLua.h"

#include "CharacterController.h"
#include "AIController.h"

#include "Log.h"

using namespace Apex::Data;
using namespace Apex::Vector3;
using namespace Apex::Hierarchy;
using namespace Apex::Controller;

void Apex::Controller::RegisterLua(Scripting::LuaManager& lua)
{
    lua.RegisterFunction("GetCharacterController_Internal", Apex::Controller::Lua_GetCharacterController);
    lua.RegisterFunction("GetAIController_Internal", Apex::Controller::Lua_GetAIController);
    lua.RegisterFunction("CC_TP_Internal", Apex::Controller::Lua_CC_TP);
    lua.RegisterFunction("CC_TPToTarget_Internal", Apex::Controller::Lua_CC_TPToTarget);
    lua.RegisterFunction("CC_Move_Internal", Apex::Controller::Lua_CC_Move);
    lua.RegisterFunction("CC_Jump_Internal", Apex::Controller::Lua_CC_Jump);
    lua.RegisterFunction("CC_GetVelocity_Internal", Apex::Controller::Lua_CC_GetVelocity);
    lua.RegisterFunction("CC_GetVerticalVelocity_Internal", Apex::Controller::Lua_CC_GetVerticalVelocity);
    lua.RegisterFunction("CC_IsGrounded_Internal", Apex::Controller::Lua_CC_IsGrounded);
    lua.RegisterFunction("CC_IsInWater_Internal", Apex::Controller::Lua_CC_IsInWater);
    lua.RegisterFunction("AIC_SetTarget_Internal", Apex::Controller::Lua_AIC_SetTarget);
    lua.RegisterFunction("CC_SetVelocityX_Internal", Apex::Controller::Lua_CC_SetVelocityX);
    lua.RegisterFunction("CC_SetVelocityY_Internal", Apex::Controller::Lua_CC_SetVelocityY);
    lua.RegisterFunction("CC_SetVelocityZ_Internal", Apex::Controller::Lua_CC_SetVelocityZ);
    lua.RegisterFunction("CC_SetVelocity_Internal", Apex::Controller::Lua_CC_SetVelocity);
}

int Apex::Controller::Lua_CC_TP(lua_State* L)
{
    CharacterController* cc = Comp::GetComponent<CharacterController>(L, 1);

    if (!cc)
        return 0;

    LuaVector3* pos = CheckVector3(L, 2);

    cc->Teleport(pos->m_value);

    return 0;
}

int Apex::Controller::Lua_CC_TPToTarget(lua_State* L)
{
    CharacterController* cc = Comp::GetComponent<CharacterController>(L, 1);

    if (!cc)
        return 0;

    Object* obj = GetObject(L, 2);
    if (!obj)
        return 0;

    cc->Teleport(obj->GetGlobalTransform());

    return 0;
}

int Apex::Controller::Lua_CC_Move(lua_State* L)
{
    CharacterController* cc = Comp::GetComponent<CharacterController>(L, 1);

    if (!cc)
        return 0;

    LuaVector3* dir = CheckVector3(L, 2);

    cc->Move(dir->m_value);

    return 0;
}

int Apex::Controller::Lua_CC_Jump(lua_State* L)
{
    CharacterController* cc = Comp::GetComponent<CharacterController>(L, 1);

    if (!cc)
        return 0;

    cc->Jump();

    return 0;
}

int Apex::Controller::Lua_CC_GetVelocity(lua_State* L)
{
    CharacterController* cc = Comp::GetComponent<CharacterController>(L, 1);

    if (!cc) return 0;

    lua_pushnumber(L, cc->GetHorizontalVelocity());

    return 1;
}

int Apex::Controller::Lua_CC_GetVerticalVelocity(lua_State* L)
{
    CharacterController* cc = Comp::GetComponent<CharacterController>(L, 1);

    if (!cc) return 0;

    lua_pushnumber(L, cc->GetVerticalVelocity());

    return 1;
}

int Apex::Controller::Lua_CC_IsGrounded(lua_State* L)
{
    CharacterController* cc = Comp::GetComponent<CharacterController>(L, 1);

    if (!cc) return 0;

    lua_pushboolean(L, cc->IsGrounded());

    return 1;
}

int Apex::Controller::Lua_CC_IsInWater(lua_State* L)
{
    CharacterController* cc = Comp::GetComponent<CharacterController>(L, 1);

    if (!cc) return 0;

    lua_pushboolean(L, cc->IsInWater());

    return 1;
}

int Apex::Controller::Lua_AIC_SetTarget(lua_State* L)
{
    AIController* AI = Comp::GetComponent<AIController>(L, 1);
    if (!AI)
        return 0;

    Object* target = nullptr;

    target = static_cast<Object*>(lua_touserdata(L, 2));
    
    if (!target)
        return 0;

    AI->SetTarget(target);

    return 0;
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

int Apex::Controller::Lua_GetAIController(lua_State* L)
{
    Object* obj = GetObject(L, 1);
    if (!obj)
        return 0;

    AIController* cc = obj->GetComponent<AIController>();
    if (!cc)
        return 0;

    // push component as userdata
    lua_pushlightuserdata(L, cc);

    return 1;
}

int Apex::Controller::Lua_CC_SetVelocityX(lua_State* L)
{
    CharacterController* cc = Comp::GetComponent<CharacterController>(L, 1);

    if (!cc)
        return 0;

    float value = (float)luaL_checknumber(L, 2);

    cc->SetVelocityX(value);

    return 1;
}

int Apex::Controller::Lua_CC_SetVelocityY(lua_State* L)
{
    CharacterController* cc = Comp::GetComponent<CharacterController>(L, 1);

    if (!cc)
        return 0;

    float value = (float)luaL_checknumber(L, 2);

    cc->SetVelocityY(value);

    return 1;
}

int Apex::Controller::Lua_CC_SetVelocityZ(lua_State* L)
{
    CharacterController* cc = Comp::GetComponent<CharacterController>(L, 1);

    if (!cc)
        return 0;

    float value = (float)luaL_checknumber(L, 2);

    cc->SetVelocityZ(value);

    return 1;
}

int Apex::Controller::Lua_CC_SetVelocity(lua_State* L)
{
    CharacterController* cc = Comp::GetComponent<CharacterController>(L, 1);

    if (!cc)
        return 0;

    LuaVector3* velocity = CheckVector3(L, 2);

    cc->SetVelocity(velocity->m_value);

    return 1;
}

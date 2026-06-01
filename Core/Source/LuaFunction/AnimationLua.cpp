#include "LuaFunction/AnimationLua.h"
#include "LuaFunction/ObjectLua.h"
#include "LuaFunction/ComponentLua.h"

#include "Animator.h"

#include "Object.h"

#include "Log.h"

using namespace Apex::Data;
using namespace Apex::Rendering;
using namespace Apex::Anim;

void Apex::Anim::RegisterLua(Scripting::LuaManager& lua)
{
    lua.RegisterFunction("GetAnimator_Internal", Apex::Anim::Lua_GetAnimator);
    lua.RegisterFunction("Anim_Play_Internal", Apex::Anim::Lua_Anim_Play);
    lua.RegisterFunction("Anim_Blend_Internal", Apex::Anim::Lua_Anim_Blend);
    lua.RegisterFunction("Anim_SetBlendRatio_Internal", Apex::Anim::Lua_Anim_SetBlendRatio);
    lua.RegisterFunction("Anim_Crossfade_Internal", Apex::Anim::Lua_Anim_Crossfade);
}

int Apex::Anim::Lua_Anim_Play(lua_State* L)
{
    Animator* animator = Comp::GetComponent<Animator>(L, 1);

    if (!animator) 
    {
        LOG_ERROR_CAT("Scripting", "couldnt get the animator in Animator::Play()");
        return 0;
    }

    const char* animName = luaL_checkstring(L, 2);

    if (!animName)
    {
        LOG_ERROR_CAT("Scripting", "Animator::Play() requires a str");
        return 0;
    }

    int idx = animator->GetModel()->GetAnimationIdx(animName);

    if (idx == -1) 
    {
        LOG_WARNING_CAT("Animation", "Animation \"{}\" not found", animName);
        return 0;
    }

    bool interpole = lua_toboolean(L, 3);

    if (interpole)
        animator->PlayRepeatInterpolated(idx);
    else
        animator->PlayRepeat(idx);

    return 0;
}

int Apex::Anim::Lua_Anim_Blend(lua_State* L)
{
    Animator* animator = Comp::GetComponent<Animator>(L, 1);

    if (!animator)
    {
        LOG_ERROR_CAT("Scripting", "couldnt get the animator in Animator::Blend()");
        return 0;
    }

    const char* animAName = luaL_checkstring(L, 2);
    const char* animBName = luaL_checkstring(L, 3);

    int idxA = animator->GetModel()->GetAnimationIdx(animAName);
    if (idxA == -1)
    {
        LOG_WARNING_CAT("Animation", "Animation \"{}\" not found", animAName);
        return 0;
    }
    
    int idxB = animator->GetModel()->GetAnimationIdx(animBName);
    if (idxB == -1)
    {
        LOG_WARNING_CAT("Animation", "Animation \"{}\" not found", animBName);
        return 0;
    }

    float ratio = lua_tonumber(L, 4);
    bool interpole = lua_toboolean(L, 5);

    if (interpole)
        animator->PlayBlendedInterpolated(idxA, idxB, ratio);
    else
        animator->PlayBlended(idxA, idxB, ratio);

    return 0;
}

int Apex::Anim::Lua_Anim_SetBlendRatio(lua_State* L)
{
    Animator* animator = Comp::GetComponent<Animator>(L, 1);

    if (!animator)
    {
        LOG_ERROR_CAT("Scripting", "couldnt get the animator in Animator::SetBlendRatio()");
        return 0;
    }

    float ratio = lua_tonumber(L, 2);

    animator->SetBlendRatio(ratio);

    return 0;
}

int Apex::Anim::Lua_Anim_Crossfade(lua_State* L)
{
    Animator* animator = Comp::GetComponent<Animator>(L, 1);

    if (!animator)
    {
        LOG_ERROR_CAT("Scripting", "couldnt get the animator in Animator::CrossFade()");
        return 0;
    }

    const char* animAName = luaL_checkstring(L, 2);
    const char* animBName = luaL_checkstring(L, 3);

    int idxA = animator->GetModel()->GetAnimationIdx(animAName);
    if (idxA == -1)
    {
        LOG_WARNING_CAT("Animation", "Animation \"{}\" not found", animAName);
        return 0;
    }

    int idxB = animator->GetModel()->GetAnimationIdx(animBName);
    if (idxB == -1)
    {
        LOG_WARNING_CAT("Animation", "Animation \"{}\" not found", animBName);
        return 0;
    }

    float time = lua_tonumber(L, 4);
    bool interpole = lua_toboolean(L, 5);

    if (interpole)
        animator->PlayCrossFadeInterpolated(idxA, idxB, time);
    else
        animator->PlayCrossFade(idxA, idxB, time);

    return 0;
}

int Apex::Anim::Lua_GetAnimator(lua_State* L)
{
    Object* obj = GetObject(L, 1);
    if (!obj)
        return 0;

    Animator* animator = obj->GetComponent<Animator>();
    if (!animator)
        return 0;

    // push component as userdata
    lua_pushlightuserdata(L, animator);

    return 1;
}

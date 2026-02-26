#include "LuaBehaviour.h"

#include <lua.hpp>

Apex::Scripting::LuaBehaviour::~LuaBehaviour()
{
    if (m_instanceRef != LUA_NOREF)
        luaL_unref(m_manager->GetState(), LUA_REGISTRYINDEX, m_instanceRef);
}

void Apex::Scripting::LuaBehaviour::Start()
{
    m_manager->CallFunction(m_instanceRef, "Increment");
}

void Apex::Scripting::LuaBehaviour::Update(float dt)
{
    m_manager->CallFunction(m_instanceRef, "Increment", dt);
}
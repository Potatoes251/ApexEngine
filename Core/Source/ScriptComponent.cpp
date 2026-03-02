#include "ScriptComponent.h"

Apex::Scripting::ScriptComponent::~ScriptComponent()
{
    if (m_instanceRef != LUA_REFNIL)
    {
        luaL_unref(m_lua.GetState(), LUA_REGISTRYINDEX, m_instanceRef);
    }
}

void Apex::Scripting::ScriptComponent::SetScript(const std::string& path)
{
    m_scriptPath = path;
    m_scriptRef = m_lua.LoadScript(path);
    m_instanceRef = m_lua.CreateScriptInstance(m_scriptRef);
}

void Apex::Scripting::ScriptComponent::OnStart()
{
    if (m_instanceRef != LUA_REFNIL)
        m_lua.CallFunction(m_instanceRef, "Start");
}

void Apex::Scripting::ScriptComponent::OnUpdate(float deltaTime)
{
    if (!IsEnabled())
        return;

    if (m_instanceRef != LUA_REFNIL)
        m_lua.CallFunction(m_instanceRef, "Update", deltaTime);
}

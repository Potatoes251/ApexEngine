#include "ScriptComponent.h"

Apex::Scripting::ScriptComponent::~ScriptComponent()
{
    if (m_instanceRef != LUA_REFNIL)
    {
        luaL_unref(m_lua.GetState(), LUA_REGISTRYINDEX, m_instanceRef);
    }
}

std::unique_ptr<Apex::Component> Apex::Scripting::ScriptComponent::Clone() const
{
    auto clone = std::make_unique<ScriptComponent>(m_lua);
    clone->SetScript(m_scriptPath);
    return clone;
}

void Apex::Scripting::ScriptComponent::SetScript(const std::string& path)
{
    m_scriptPath = path;
    m_scriptRef = m_lua.LoadScript(path);
    m_instanceRef = m_lua.CreateScriptInstance(m_scriptRef);

    lua_State* L = m_lua.GetState();
    lua_rawgeti(L, LUA_REGISTRYINDEX, m_instanceRef); // push instance

    // push light userdata to Object*
    lua_pushlightuserdata(L, GetOwner());

    // instance.__object = pointer
    lua_setfield(L, -2, "__object");

    // pop instance
    lua_pop(L, 1);
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

void Apex::Scripting::ScriptComponent::Serialize(std::ostream& out) const
{
    out << "      \"ScriptComponent\": {\n";
    out << "        \"enabled\": " << (IsEnabled() ? "true" : "false") << ",\n";
    out << "        \"script\": \"" << m_scriptPath << "\"\n";
    out << "      }";
}
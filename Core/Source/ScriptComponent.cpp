#include "ScriptComponent.h"

using namespace Apex;
using namespace Apex::Scripting;

ScriptComponent::~ScriptComponent()
{
    if (m_instanceRef != LUA_REFNIL)
    {
        luaL_unref(m_lua.GetState(), LUA_REGISTRYINDEX, m_instanceRef);
    }
}

std::unique_ptr<Apex::Component> ScriptComponent::Clone() const
{
    auto clone = std::make_unique<ScriptComponent>(m_lua);
    clone->SetScript(m_scriptPath);
    return clone;
}

void ScriptComponent::SetScript(const std::string& path)
{
    m_scriptPath = path;
    m_scriptRef = m_lua.LoadScript(path);
    m_instanceRef = m_lua.CreateScriptInstance(m_scriptRef);

    m_instanceRef = m_lua.CreateScriptInstance(m_scriptRef);

    // After instance creation:
    lua_State* L = m_lua.GetState();
    lua_rawgeti(L, LUA_REGISTRYINDEX, m_instanceRef); // push instance

    // push light userdata to Object*
    lua_pushlightuserdata(L, GetOwner());

    // instance.__object = pointer
    lua_setfield(L, -2, "__object");

    // pop instance
    lua_pop(L, 1);
}

std::vector<ExposedVar> ScriptComponent::GetExposedVariables()
{
    return std::vector<ExposedVar>();
}

void ScriptComponent::DrawVariables(Apex::UserInterface::IGUI& gui)
{
    
}

void ScriptComponent::OnStart()
{
    if (m_instanceRef != LUA_REFNIL)
        m_lua.CallFunction(m_instanceRef, "Start");
}

void ScriptComponent::OnUpdate(float deltaTime)
{
    if (!IsEnabled())
        return;

    if (m_instanceRef != LUA_REFNIL)
        m_lua.CallFunction(m_instanceRef, "Update", deltaTime);
}

void ScriptComponent::Serialize(std::ostream& out) const
{
    out << "      \"ScriptComponent\": {\n";
    out << "        \"enabled\": " << (IsEnabled() ? "true" : "false") << ",\n";
    out << "        \"script\": \"" << m_scriptPath << "\"\n";
    out << "      }";
}
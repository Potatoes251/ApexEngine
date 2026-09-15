#include "ScriptComponent.h"

using namespace Apex;
using namespace Apex::Scripting;

std::unique_ptr<Apex::Component> ScriptComponent::Clone()
{
    auto clone = std::make_unique<ScriptComponent>(m_lua);
    clone->SetEnabled(IsEnabled());
    clone->SetScript(m_scriptPath);

    for (auto& var : GetExposedVariables())
    {
        clone->SetVariable(var.m_name, const_cast<ExposedVar&>(var));
    }

    return clone;
}

void ScriptComponent::SetScript(const std::string& path)
{
    if (m_scriptPath == path && m_instanceRef != LUA_REFNIL)
        return;

    m_scriptPath = path;

    if (path.empty()) 
    {
        m_instanceRef = LUA_REFNIL;
        return;
    }

    m_scriptRef = m_lua->LoadScript(path);
    m_instanceRef = m_lua->CreateScriptInstance(m_scriptRef);

    lua_State* L = m_lua->GetState();
    lua_rawgeti(L, LUA_REGISTRYINDEX, m_instanceRef); // push instance

    // push light userdata to Object*
    lua_pushlightuserdata(L, GetOwner());

    // instance.__object = pointer
    lua_setfield(L, -2, "__object");

    lua_pushlightuserdata(L, static_cast<Component*>(this));
    lua_setfield(L, -2, "__component");

    // pop instance
    lua_pop(L, 1);
}

void Apex::Scripting::ScriptComponent::SetObjectPtr()
{
    if (m_instanceRef == LUA_REFNIL)
        return;

    lua_State* L = m_lua->GetState();

    // Push instance
    lua_rawgeti(L, LUA_REGISTRYINDEX, m_instanceRef);

    // Push new pointer
    lua_pushlightuserdata(L, GetOwner());

    // instance.__object = new pointer
    lua_setfield(L, -2, "__object");

    // Pop instance
    lua_pop(L, 1);
}

std::vector<ExposedVar> ScriptComponent::GetExposedVariables()
{
    std::vector<ExposedVar> vars;

    if (m_instanceRef == LUA_NOREF || m_instanceRef == LUA_REFNIL)
        return vars;

    lua_State* L = m_lua->GetState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, m_instanceRef);

    if (!lua_istable(L, -1))
    {
        lua_pop(L, 1);
        return vars;
    }

    lua_getfield(L, -1, "__exposed");

    if (!lua_istable(L, -1))
    {
        lua_pop(L, 2);
        return vars;
    }

    int len = (int)lua_objlen(L, -1);

    for (int i = 1; i <= len; i++)
    {
        lua_rawgeti(L, -1, i);

        if (!lua_isstring(L, -1))
        {
            lua_pop(L, 1);
            continue;
        }

        std::string name = lua_tostring(L, -1);
        lua_pop(L, 1);

        // get value in instance
        lua_getfield(L, -2, name.c_str());

        int type = lua_type(L, -1);

        ExposedVar var;
        var.m_name = name;

        switch (type)
        {
        case LUA_TNUMBER:
        {
            var.type = ExposedVar::Float;
            var.m_value = (float)lua_tonumber(L, -1);
            vars.push_back(var);
            break;
        }

        case LUA_TBOOLEAN:
        {
            var.type = ExposedVar::Bool;
            var.m_value = (bool)lua_toboolean(L, -1);
            vars.push_back(var);
            break;
        }

        case LUA_TSTRING:
        {
            var.type = ExposedVar::String;
            var.m_value = std::string(lua_tostring(L, -1));
            vars.push_back(var);
            break;
        }
        }

        lua_pop(L, 1); // pop value
    }

    lua_pop(L, 2); // __exposed + instance

    return vars;
}

void ScriptComponent::SetVariable(const std::string& name, ExposedVar& var)
{
    if (m_instanceRef == LUA_REFNIL)
        return;

    lua_State* L = m_lua->GetState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, m_instanceRef); // push instance

    switch (var.type)
    {
    case ExposedVar::Float:
        lua_pushnumber(L, std::get<float>(var.m_value));
        break;
    case ExposedVar::Bool:
        lua_pushboolean(L, std::get<bool>(var.m_value));
        break;
    case ExposedVar::String:
        lua_pushstring(L, std::get<std::string>(var.m_value).c_str());
        break;
    default:
        lua_pop(L, 1);
        return;
    }

    lua_setfield(L, -2, name.c_str()); // instance[name] = value

    lua_pop(L, 1); // pop instance
}

void ScriptComponent::SetVariable(const std::string& name, float var)
{
    if (m_instanceRef == LUA_REFNIL)
        return;

    lua_State* L = m_lua->GetState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, m_instanceRef); // push instance

    lua_pushnumber(L, var);

    lua_setfield(L, -2, name.c_str()); // instance[name] = value

    lua_pop(L, 1); // pop instance
}

void ScriptComponent::SetVariable(const std::string& name, bool var)
{
    if (m_instanceRef == LUA_REFNIL)
        return;

    lua_State* L = m_lua->GetState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, m_instanceRef); // push instance

    lua_pushboolean(L, var);

    lua_setfield(L, -2, name.c_str()); // instance[name] = value

    lua_pop(L, 1); // pop instance
}

void Apex::Scripting::ScriptComponent::SetVariable(const std::string& name, std::string const& var)
{
    if (m_instanceRef == LUA_REFNIL)
        return;

    lua_State* L = m_lua->GetState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, m_instanceRef); // push instance

    lua_pushstring(L, var.c_str());

    lua_setfield(L, -2, name.c_str()); // instance[name] = value

    lua_pop(L, 1); // pop instance
}

void ScriptComponent::OnStart()
{
    SetObjectPtr();
    if (m_instanceRef != LUA_REFNIL)
        m_lua->CallFunction(m_instanceRef, "Start");
}

void ScriptComponent::OnUpdate(float deltaTime)
{
    if (m_instanceRef != LUA_REFNIL)
    {
        m_lua->CallFunction(m_instanceRef, "Update", deltaTime);
    }
}

void ScriptComponent::OnFixedUpdate(float deltaTime)
{
    if (m_instanceRef != LUA_REFNIL)
    {
        m_lua->CallFunction(m_instanceRef, "FixedUpdate", deltaTime);
    }
}

void ScriptComponent::OnTriggerEnter(Physic::Collider* other)
{
    if (m_instanceRef != LUA_REFNIL)
        m_lua->CallFunction(m_instanceRef, "OnTriggerEnter", other);
}

void ScriptComponent::OnTriggerExit(Physic::Collider* other)
{
    if (m_instanceRef != LUA_REFNIL)
        m_lua->CallFunction(m_instanceRef, "OnTriggerExit", other);
}

void ScriptComponent::Serialize(std::ostream& out) const
{
    out << "        \"script\": \"" << m_scriptPath << "\",\n";

    auto vars = const_cast<ScriptComponent*>(this)->GetExposedVariables();

    out << "        \"vars\": {\n";

    for (size_t i = 0; i < vars.size(); i++)
    {
        const auto& var = vars[i];

        out << "          \"" << var.m_name << "\": ";

        switch (var.type)
        {
        case ExposedVar::Float:
            out << std::get<float>(var.m_value);
            break;

        case ExposedVar::Bool:
            out << (std::get<bool>(var.m_value) ? "true" : "false");
            break;

        case ExposedVar::String:
            out << "\"" << std::get<std::string>(var.m_value) << "\"";
            break;
        }

        if (i < vars.size() - 1)
            out << ",";

        out << "\n";
    }

    out << "        }\n";
}
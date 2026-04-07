#include "ScriptComponent.h"

using namespace Apex;
using namespace Apex::Scripting;

std::unique_ptr<Apex::Component> ScriptComponent::Clone()
{
    auto clone = std::make_unique<ScriptComponent>(m_lua);
    clone->SetScript(m_scriptPath);

    for (auto& var : GetExposedVariables())
    {
        clone->SetVariable(var.name, const_cast<ExposedVar&>(var));
    }

    return clone;
}

void ScriptComponent::SetScript(const std::string& path)
{
    if (m_scriptPath == path && m_instanceRef != LUA_REFNIL)
        return;

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

std::vector<ExposedVar> ScriptComponent::GetExposedVariables()
{
    std::vector<ExposedVar> vars;

    if (m_instanceRef == LUA_NOREF || m_instanceRef == LUA_REFNIL)
        return vars;

    lua_State* L = m_lua.GetState();

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
        var.name = name;

        switch (type)
        {
        case LUA_TNUMBER:
        {
            float* value = new float((float)lua_tonumber(L, -1));
            var.type = ExposedVar::Float;
            var.data = value;
            vars.push_back(var);
            break;
        }

        case LUA_TBOOLEAN:
        {
            bool* value = new bool(lua_toboolean(L, -1));
            var.type = ExposedVar::Bool;
            var.data = value;
            vars.push_back(var);
            break;
        }

        case LUA_TSTRING:
        {
            const char* str = lua_tostring(L, -1);

            char* buffer = new char[256];
            strncpy(buffer, str ? str : "", 255);
            buffer[255] = '\0';

            var.type = ExposedVar::String;
            var.data = buffer;
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

    printf("SET instance: %d\n", m_instanceRef);

    lua_State* L = m_lua.GetState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, m_instanceRef); // push instance

    switch (var.type)
    {
    case ExposedVar::Float:
        lua_pushnumber(L, *(float*)var.data);
        break;

    case ExposedVar::Int:
        lua_pushinteger(L, *(int*)var.data);
        break;

    case ExposedVar::Bool:
        lua_pushboolean(L, *(bool*)var.data);
        break;

    case ExposedVar::String:
        lua_pushstring(L, (char*)var.data);
        break;

    default:
        lua_pop(L, 1);
        return;
    }

    lua_setfield(L, -2, name.c_str()); // instance[name] = value

    lua_pop(L, 1); // pop instance
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
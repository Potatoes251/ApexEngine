#ifndef SCRIPT_COMPONENT
#define SCRIPT_COMPONENT

#include "Component.h"
#include "LuaManager.h"

#include <string>

#include <lua.hpp>

namespace Apex::Scripting
{
    class ScriptComponent : public Component
    {
    public:
        ScriptComponent(LuaManager& lua) : m_lua(lua) {};
        ~ScriptComponent() = default;

        std::unique_ptr<Component> Clone() override;

        void SetScript(const std::string& path);
        const std::string GetScriptPath() const { return m_scriptPath; }

        const char* GetTypeName() const override { return "ScriptComponent"; };
        std::vector<ExposedVar> GetExposedVariables() override;

        void SetVariable(const std::string& name, ExposedVar& var);

        int GetInstanceRef() const { return m_instanceRef; }

        void Serialize(std::ostream& out) const override;

    protected:
        void OnStart() override;
        void OnUpdate(float deltaTime) override;

    private:
        int m_instanceRef = LUA_REFNIL;
        int m_scriptRef = LUA_REFNIL;
        Apex::Scripting::LuaManager& m_lua;
        std::string m_scriptPath;
    };
}

#endif // SCRIPT_COMPONENT
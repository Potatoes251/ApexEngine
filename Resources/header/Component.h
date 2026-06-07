#ifndef COMPONENT
#define COMPONENT

#include "../../UI/Header/UI.h"

#include <ostream>
#include <vector>
#include <variant>

namespace Apex::Data { class Object; }
namespace Apex::Physic { class Collider; }

namespace Apex
{
    struct ExposedVar
    {
        enum Type { Float, Int, Bool, String, Enum, Vector3, Mesh, Texture, Material, Color3, Color4 } type;

        ExposedVar() = default;

        ExposedVar(std::string name, Type type, void* data)
            : m_name(std::move(name)), type(type), m_data(data) {
        }

        ExposedVar(std::string name, Type type, void* data, std::vector<std::string> enumNames)
            : m_name(std::move(name)), type(type), m_data(data), m_enumNames(std::move(enumNames)) {
        }

        std::string m_name;
        void* m_data = nullptr;
        std::variant<float, bool, std::string> m_value;
        std::vector<std::string> m_enumNames;
    };  

    class Component
    {
    public:
        Component() = default;
        virtual ~Component() = default;

        Component(const Component&) = default;
        Component& operator=(const Component&) = default;

        virtual void OnStart() {}
        virtual void OnDestroy() {}
        virtual void OnUpdate(float deltatime_s) {}
        virtual void OnLateUpdate(float deltatime_s) {}
        virtual void OnFixedUpdate(float fixedDeltaTime_s) {}
        virtual void OnTriggerEnter(Physic::Collider* other) {}
        virtual void OnTriggerExit(Physic::Collider* other) {}

        virtual void Serialize(std::ostream& out) const = 0;

        virtual std::unique_ptr<Component> Clone() = 0;

        virtual const char* GetTypeName() const = 0;
        virtual std::vector<ExposedVar> GetExposedVariables() = 0;

        void SetEnabled(bool enabled) { m_enabled = enabled; }
        bool IsEnabled() const { return m_enabled; }

        Apex::Data::Object* GetOwner() const { return m_owner; }

        size_t  GetId() const { return m_id; }
        void    SetId(size_t newId) { m_id = newId; }

    private:
        friend class Apex::Data::Object;
        Apex::Data::Object* m_owner = nullptr;

        bool m_enabled = true;

        size_t m_id = SIZE_MAX;
    };
}

#endif // COMPONENT
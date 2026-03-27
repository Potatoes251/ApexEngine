#ifndef COMPONENT
#define COMPONENT

#include "../../UI/Header/UI.h"

#include <ostream>
#include <vector>

namespace Apex::Data { class Object; }

namespace Apex
{
    struct ExposedVar
    {
        std::string name;
        enum Type { Float, Int, Bool, String, Enum, Vector3, PhysMaterial, Mesh, Texture, Script } type;
        void* data;
        std::vector<std::string> enumNames;
    };  

    class Component
    {
    public:
        Component() = default;
        virtual ~Component() = default;

        Component(const Component&) = delete;
        Component& operator=(const Component&) = delete;

        virtual void OnStart() {}
        virtual void OnDestroy() {}
        virtual void OnUpdate(float deltaTime) {}
        virtual void OnFixedUpdate(float fixedDeltaTime) {}
        virtual void Serialize(std::ostream& out) const = 0;

        virtual std::unique_ptr<Component> Clone() const = 0;

        virtual const char* GetTypeName() const = 0;
        virtual std::vector<ExposedVar> GetExposedVariables() = 0;

        void SetEnabled(bool enabled) { m_enabled = enabled; }
        bool IsEnabled() const { return m_enabled; }

        Apex::Data::Object* GetOwner() const { return m_owner; }

        size_t GetId() const { return m_id; }
        void SetId(size_t newId) { m_id = newId; }

    private:
        friend class Apex::Data::Object;

        Apex::Data::Object* m_owner = nullptr;
        bool m_enabled = true;

        size_t m_id = SIZE_MAX;
    };
}

#endif // COMPONENT
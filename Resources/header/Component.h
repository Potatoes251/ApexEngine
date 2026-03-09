#ifndef COMPONENT
#define COMPONENT

namespace Apex::Data { class Object; }

namespace Apex
{
    class Component
    {
    public:
        Component() = default;
        virtual ~Component() = default;

        Component(const Component&) = delete;
        Component& operator=(const Component&) = delete;

        virtual void OnStart() {}
        virtual void OnUpdate(float deltaTime) {}
        virtual void OnDestroy() {}
        virtual void OnFixedUpdate(float fixedDeltaTime) {}

        void SetEnabled(bool enabled) { m_enabled = enabled; }
        bool IsEnabled() const { return m_enabled; }

        Apex::Data::Object* GetOwner() const { return m_owner; }

    private:
        friend class Apex::Data::Object;

        Apex::Data::Object* m_owner = nullptr;
        bool m_enabled = true;
    };
}

#endif // COMPONENT
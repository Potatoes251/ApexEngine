#ifndef PROJECTILE_COMPONENT
#define PROJECTILE_COMPONENT

#include "Component.h"
#include "LibMath/Vector/Vector3.h"

namespace Apex::Physic
{
    class PhysicSystem;
}

namespace Apex::Gameplay
{
    class ProjectileComponent : public Component
    {
    public:
        ProjectileComponent() = default;
        ProjectileComponent(ProjectileComponent const& other) = default;
        ProjectileComponent& operator=(ProjectileComponent const& other) = default;
        ~ProjectileComponent() = default;

        // Configuration setup
        void Setup(Physic::PhysicSystem* physic, LibMath::Vector3 const& direction, float speed, float lifeSpan);

        // Component Overrides
        void OnFixedUpdate(float fixedDeltaTime_s) override;

        void Serialize(std::ostream& out) const override;
        std::unique_ptr<Component> Clone() override { return std::make_unique<ProjectileComponent>(*this); }
        const char* GetTypeName() const override { return "ProjectileComponent"; }
        std::vector<ExposedVar> GetExposedVariables() override;

    private:
        Physic::PhysicSystem* m_physic = nullptr;
        LibMath::Vector3      m_direction = { 0.f, 0.f, 0.f };
        float                 m_speed = 20.f;
        float                 m_lifeSpan = 3.f;
        float                 m_age = 0.f;
    };
}

#endif
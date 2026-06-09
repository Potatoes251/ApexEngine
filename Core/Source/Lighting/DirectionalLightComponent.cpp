#include "Lighting/DirectionalLightComponent.h"

#include "GpuLightStructs.h"
#include "Object.h"
#include "Camera.h"

using namespace Apex;
using namespace Apex::Lighting;

void DirectionalLightComponent::Serialize(std::ostream& out) const
{
    out << "        \"color\": ["
        << m_color[0] << ", "
        << m_color[1] << ", "
        << m_color[2] << "],\n";
    out << "        \"intensity\": " << m_intensity << ",\n";
    out << "        \"shadowDistance\": " << m_shadowDistance << ",\n";
    out << "        \"shadowExtent\": " << m_shadowExtents << "\n";
}

void DirectionalLightComponent::FillGpuData(void* data_out)
{
    GPUDirectionalLight* data = (GPUDirectionalLight*)data_out;

    data->m_color = { m_color, 0 };
    data->m_param = { m_intensity, (float)m_shadowIdx, 0, 0 };
    data->m_direction = { GetOwner()->GetGlobalTransform().getForward(), 0 };
}

LibMath::Matrix4 DirectionalLightComponent::GetProjMatrix() const
{
    return LibMath::Matrix4::orthogonal(-m_shadowExtents, m_shadowExtents, -m_shadowExtents, m_shadowExtents, .1f, m_shadowDistance * 2.f);
}

std::vector<LibMath::Matrix4> DirectionalLightComponent::GetViewMatrix(Rendering::Camera* camera) const
{
    LibMath::Vector3 forward = GetOwner()->GetGlobalTransform().getForward();
    LibMath::Vector3 sceneCenter = camera->GetPosition() + forward * m_shadowDistance / 2.f;

    LibMath::Vector3 eye = camera->GetPosition() - forward * m_shadowDistance;

    LibMath::Vector3 worldUp = LibMath::Vector3::up();

    if (fabs(forward.dot(worldUp)) > 0.999f)
    {
        worldUp = LibMath::Vector3(0.f, 0.f, 1.f); // fallback axis
    }

    return { LibMath::Matrix4::lookAt(eye, sceneCenter, worldUp) };
}

LibMath::Matrix4 DirectionalLightComponent::GetViewMatrix(std::array<LibMath::Vector3, 8> const& sliceCorners) const
{
    // Compute center of the slice frustum
    LibMath::Vector3 center = { 0.f, 0.f, 0.f };
    for (const auto& corner : sliceCorners)
        center += corner;
    center /= 8.f;

    LibMath::Vector3 forward = GetOwner()->GetGlobalTransform().getForward().normalized();

    LibMath::Vector3 eye = center - forward; // distance doesn't matter for directional

    LibMath::Vector3 worldUp = LibMath::Vector3::up();
    if (fabs(forward.dot(worldUp)) > 0.999f)        // avoid up and forward being identical
        worldUp = LibMath::Vector3(0.f, 0.f, 1.f);

    return LibMath::Matrix4::lookAt(eye, center, worldUp);
}

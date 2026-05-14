#include "Lighting/DirectionalLightComponent.h"

#include "GpuLightStructs.h"
#include "Object.h"
#include "Camera.h"

using namespace Apex;
using namespace Apex::Lighting;

DirectionalLightComponent::DirectionalLightComponent(DirectionalLightComponent const& other)
{
    m_color = other.m_color;
    m_intensity = other.m_intensity;
}

DirectionalLightComponent& DirectionalLightComponent::operator=(DirectionalLightComponent const& other)
{
    m_color = other.m_color;
    m_intensity = other.m_intensity;

    return *this;
}

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

std::vector<ExposedVar> DirectionalLightComponent::GetExposedVariables()
{
    std::vector<ExposedVar> exposedVec = LightComponent::GetExposedVariables();

    exposedVec.push_back({ "Shadow Distance", ExposedVar::Float, &m_shadowDistance });
    exposedVec.push_back({ "Shadow Extent", ExposedVar::Float, &m_shadowExtents });

    return exposedVec;
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

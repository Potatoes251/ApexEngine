#include "Lighting/SpotLightComponent.h"

#include "GpuLightStructs.h"
#include "Object.h"

#include "LibMath/Trigonometry.h"

using namespace Apex;
using namespace Apex::Lighting;

SpotLightComponent::SpotLightComponent(LibMath::Vector3 color, float intensity, float innerCutoff, float outerCutoff)
    : LightComponent(color, intensity) 
{
    m_innerCutoff = innerCutoff;
    m_outerCutoff = outerCutoff;
}

void SpotLightComponent::Serialize(std::ostream& out) const
{
    out << "        \"color\": " << m_color << ",\n";
    out << "        \"intensity\": " << m_intensity << ",\n";
    out << "        \"innerCutOff\": " << m_innerCutoff << ",\n";
    out << "        \"outerCutOff\": " << m_outerCutoff << "\n";
}

std::vector<ExposedVar> SpotLightComponent::GetExposedVariables()
{
    std::vector<ExposedVar> exposedVec = LightComponent::GetExposedVariables();

    exposedVec.push_back({ "Near Plane Shadow", ExposedVar::Float, &m_nearShadow });
    exposedVec.push_back({ "Far Plane Shadow", ExposedVar::Float, &m_farShadow });
    exposedVec.push_back({ "Inner Angle", ExposedVar::Float, &m_innerCutoff });
    exposedVec.push_back({ "Outer Angle", ExposedVar::Float, &m_outerCutoff });

    return exposedVec;
}

void SpotLightComponent::FillGpuData(void* data_out)
{
    GPUSpotLight* data = (GPUSpotLight*)data_out;

    data->m_color = { m_color, 0 };
    data->m_direction = { GetOwner()->GetGlobalTransform().getForward(), 0 };
    data->m_position = { GetOwner()->GetGlobalTransform().getPosition(), 0 };
    data->m_param = { m_intensity, (float)m_shadowIdx, m_innerCutoff, m_outerCutoff  };
}

LibMath::Matrix4 Apex::Lighting::SpotLightComponent::GetProjMatrix() const
{
    return LibMath::Matrix4::perspective(LibMath::acos(m_outerCutoff) * 2.f, 1.f, m_nearShadow, m_farShadow);
}

std::vector<LibMath::Matrix4> Apex::Lighting::SpotLightComponent::GetViewMatrix(Rendering::Camera*) const
{
    LibMath::Vector3 pos = GetOwner()->GetGlobalTransform().getPosition();
    LibMath::Vector3 forward = GetOwner()->GetGlobalTransform().getForward();

    LibMath::Vector3 worldUp = LibMath::Vector3::up();

    if (fabs(forward.dot(worldUp)) > 0.999f)
    {
        worldUp = LibMath::Vector3(0.f, 0.f, 1.f); // fallback axis
    }

    return { LibMath::Matrix4::lookAt(pos, pos + forward, worldUp) };
}

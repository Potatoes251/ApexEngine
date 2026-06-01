#include "Lighting/PointLightComponent.h"

#include "GpuLightStructs.h"
#include "Object.h"

using namespace Apex;
using namespace Apex::Lighting;

PointLightComponent::PointLightComponent(LibMath::Vector3 color, float intensity, float radius)
    : LightComponent(color, intensity) 
{
    m_radius = radius;
}

void PointLightComponent::Serialize(std::ostream& out) const
{
    out << "        \"color\": " << m_color << ",\n";
    out << "        \"intensity\": " << m_intensity << ",\n";
    out << "        \"radius\": " << m_radius << "\n";
}

std::vector<ExposedVar> PointLightComponent::GetExposedVariables()
{
    std::vector<ExposedVar> exposedVec = LightComponent::GetExposedVariables();

    exposedVec.push_back({ "Near Plane Shadow", ExposedVar::Float, &m_nearShadow });
    exposedVec.push_back({ "Far Plane Shadow", ExposedVar::Float, &m_farShadow });
    exposedVec.push_back({ "Radius", ExposedVar::Float, &m_radius });

    return exposedVec;
}

void PointLightComponent::FillGpuData(void* data_out)
{
    GPUPointLight* data = (GPUPointLight*)data_out;

    data->m_color = { m_color, 0 };
    data->m_param = { m_intensity, (float)m_shadowIdx, m_radius, m_farShadow };
    data->m_position = { GetOwner()->GetGlobalTransform().getPosition(), 0 };
}

LibMath::Matrix4 PointLightComponent::GetProjMatrix() const
{
    return LibMath::Matrix4::perspective(LibMath::Degree(90.f), 1.f, m_nearShadow, m_farShadow);
}

std::vector<LibMath::Matrix4> PointLightComponent::GetViewMatrix(Rendering::Camera*) const
{
    std::vector<LibMath::Matrix4> matrices;

    LibMath::Vector3 position = GetOwner()->GetGlobalTransform().getPosition();

    matrices.push_back(LibMath::Matrix4::lookAt(position, position + LibMath::Vector3::right(), LibMath::Vector3::down()));
    matrices.push_back(LibMath::Matrix4::lookAt(position, position + LibMath::Vector3::left(), LibMath::Vector3::down()));
    matrices.push_back(LibMath::Matrix4::lookAt(position, position + LibMath::Vector3::up(), LibMath::Vector3::front()));
    matrices.push_back(LibMath::Matrix4::lookAt(position, position + LibMath::Vector3::down(), LibMath::Vector3::back()));
    matrices.push_back(LibMath::Matrix4::lookAt(position, position + LibMath::Vector3::front(), LibMath::Vector3::down()));
    matrices.push_back(LibMath::Matrix4::lookAt(position, position + LibMath::Vector3::back(), LibMath::Vector3::down()));

    return matrices;
}

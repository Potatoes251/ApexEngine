#include "Lighting/LightComponent.h"

using namespace Apex;
using namespace Apex::Lighting;

LightComponent::LightComponent(LibMath::Vector3 color, float intensity)
{
    m_color = color;
    m_intensity = intensity;
}

std::vector<ExposedVar> LightComponent::GetExposedVariables()
{
    std::vector<ExposedVar> exposedVec;

    exposedVec.push_back({ "Color", ExposedVar::Color3, &m_color });
    exposedVec.push_back({ "Intensity", ExposedVar::Float, &m_intensity });
    exposedVec.push_back({ "Cast Shadows", ExposedVar::Bool, &m_castShadows });

    return exposedVec;
}
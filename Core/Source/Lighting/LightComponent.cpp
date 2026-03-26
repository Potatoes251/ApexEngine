#include "Lighting/LightComponent.h"

using namespace Apex;
using namespace Apex::Lighting;

std::vector<ExposedVar> LightComponent::GetExposedVariables()
{
    std::vector<ExposedVar> exposedVec;

    exposedVec.push_back({ "Color", ExposedVar::Vector3, &m_color });
    exposedVec.push_back({ "Intensity", ExposedVar::Float, &m_intensity });

    return exposedVec;
}
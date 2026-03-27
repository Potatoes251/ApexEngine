#include "Lighting/DirectionalLightComponent.h"

#include "Light.h"
#include "Object.h"

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
    out << "      \"DirectionalLight\": {\n";

    out << "        \"enabled\": " << (IsEnabled() ? "true" : "false") << ",\n";
    out << "        \"color\": ["
        << m_color[0] << ", "
        << m_color[1] << ", "
        << m_color[2] << "],\n";
    out << "        \"intensity\": " << m_intensity << "\n";

    out << "      }";
}

std::vector<ExposedVar> DirectionalLightComponent::GetExposedVariables()
{
    return LightComponent::GetExposedVariables();
}

void DirectionalLightComponent::FillGpuData(void* data_out)
{
    GPUDirectionalLight* data = (GPUDirectionalLight*)data_out;

    data->m_color = { m_color, 0 };
    data->m_param = { m_intensity, 0, 0, 0 };
    data->m_direction = { GetOwner()->GetGlobalTransform().getForward(), 0 };
}

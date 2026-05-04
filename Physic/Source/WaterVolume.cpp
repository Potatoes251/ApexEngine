#include "WaterVolume.h"

using namespace Apex::Physic;

void WaterVolume::Serialize(std::ostream& out) const
{
    out << "      \"WaterVolume\": {\n";

    out << "        \"enabled\": " << (IsEnabled() ? "true" : "false") << ",\n";

    out << "        \"Density\": " << m_density << ",\n";
    out << "        \"Drag\": " << m_drag << ",\n";
    out << "        \"Buoyancy Multiplier\": " << m_buoyancyMultiplier << ",\n";
    out << "        \"Flow Direction\": " << m_flowDirection << ",\n";
    out << "        \"Flow Speed\": " << m_flowSpeed << "\n";

    out << "      }";
}

std::vector<Apex::ExposedVar> WaterVolume::GetExposedVariables()
{
    return
    {
        { "Density", ExposedVar::Float, &m_density },
        { "Drag", ExposedVar::Float, &m_drag },
        { "Buoyancy Multiplier", ExposedVar::Float, &m_buoyancyMultiplier },
        { "Flow Direction", ExposedVar::Vector3, &m_flowDirection },
        { "Flow Speed", ExposedVar::Float, &m_flowSpeed },
    };
}

#include "WaterVolume.h"

#include "Object.h"

using namespace Apex::Physic;

WaterVolume::WaterVolume(LibMath::Vector3 halfExtents, float density, float drag, float buoyancyMultiplier, LibMath::Vector3 flowDir, float flowSpeed)
    : m_halfExtents(halfExtents), m_density(density), m_drag(drag), m_buoyancyMultiplier(buoyancyMultiplier),
    m_flowDirection(flowDir), m_flowSpeed(flowSpeed) 
{}

void WaterVolume::Serialize(std::ostream& out) const
{
    out << "        \"halfExtents\": " << m_halfExtents << ",\n";
    out << "        \"density\": " << m_density << ",\n";
    out << "        \"drag\": " << m_drag << ",\n";
    out << "        \"buoyancy Multiplier\": " << m_buoyancyMultiplier << ",\n";
    out << "        \"flow Direction\": " << m_flowDirection << ",\n";
    out << "        \"flow Speed\": " << m_flowSpeed << "\n";
}

std::vector<Apex::ExposedVar> WaterVolume::GetExposedVariables()
{
    return
    {
        { "Half Extents", ExposedVar::Vector3, &m_halfExtents },
        { "Density", ExposedVar::Float, &m_density },
        { "Drag", ExposedVar::Float, &m_drag },
        { "Buoyancy Multiplier", ExposedVar::Float, &m_buoyancyMultiplier },
        { "Flow Direction", ExposedVar::Vector3, &m_flowDirection },
        { "Flow Speed", ExposedVar::Float, &m_flowSpeed },
    };
}

bool WaterVolume::IsInside(LibMath::Vector3 const& pos) const
{
    LibMath::Vector3 volPos = GetOwner()->GetGlobalPosition();

    return (pos[0] <= volPos[0] + m_halfExtents[0] &&
            pos[1] <= volPos[1] + m_halfExtents[1] &&
            pos[2] <= volPos[2] + m_halfExtents[2] &&
            pos[0] >= volPos[0] - m_halfExtents[0] &&
            pos[1] >= volPos[1] - m_halfExtents[1] &&
            pos[2] >= volPos[2] - m_halfExtents[2]);
}

#include "Waypoint.h"


using namespace Apex::Pathfinding;

void NavGenVolume::Serialize(std::ostream& out) const
{
	out << "        \"spacing\": " << m_spacing << ",\n";
	out << "        \"genHeight\": " << m_genHeight << "\n";
}

std::vector<Apex::ExposedVar> NavGenVolume::GetExposedVariables()
{
	return {
		{ "Spacing", ExposedVar::Float, &m_spacing },
		{ "Generation Height", ExposedVar::Float, &m_genHeight },
	};
}

LibMath::Vector3 NavGenVolume::GetMinBound()
{
	LibMath::Vector3 pos = GetOwner()->GetGlobalPosition();
	LibMath::Vector3 scale = GetOwner()->GetGlobalScale();
	return pos - scale / 2.f;
}

LibMath::Vector3 NavGenVolume::GetMaxBound()
{
	LibMath::Vector3 pos = GetOwner()->GetGlobalPosition();
	LibMath::Vector3 scale = GetOwner()->GetGlobalScale();
	return pos + scale / 2.f;
}

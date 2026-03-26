#ifndef LIGHT
#define LIGHT

#include "LibMath/Vector/Vector3.h"

// structs to hold gpu datas

namespace Apex::Lighting
{
	struct GPUPointLight
	{
		LibMath::Vector3 m_position;
		LibMath::Vector3 m_color;
		float m_intensity;
		float m_radius;
	};

	struct GPUDirectionalLight
	{
		LibMath::Vector3 m_direction;
		LibMath::Vector3 m_color;
		float m_intensity;
	};

	struct GPUSpotLight
	{
		LibMath::Vector3 m_position;
		LibMath::Vector3 m_direction;
		LibMath::Vector3 m_color;

		float m_intensity;

		float m_innerCutoff;
		float m_outerCutoff;
	};
}

#endif // !LIGHT


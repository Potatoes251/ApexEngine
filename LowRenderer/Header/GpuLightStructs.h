#ifndef LIGHT
#define LIGHT

#include "LibMath/Vector/Vector4.h"

// structs to hold gpu datas
// use only vec4 to avoid padding mismatch between cpu and gpu structs

namespace Apex::Lighting
{
	constexpr size_t MAX_POINT_LIGHTS = 10;
	constexpr size_t MAX_DIR_LIGHTS = 10;
	constexpr size_t MAX_SPOT_LIGHTS = 10;

	struct alignas(16) GPUPointLight
	{
		LibMath::Vector4 m_position;
		LibMath::Vector4 m_color;
		LibMath::Vector4 m_param; // x = intensity, y = shadowIdx, z = radius, w = farPlane
	};

	struct alignas(16) GPUPointLightBuffer
	{
		LibMath::Vector4 m_count;
		GPUPointLight m_lights[MAX_POINT_LIGHTS];
	};

	struct alignas(16) GPUDirectionalLight
	{
		LibMath::Vector4 m_direction;
		LibMath::Vector4 m_color;
		LibMath::Vector4 m_param; // x = intensity, y = shadowIdx
	};

	struct alignas(16) GPUDirectionalLightBuffer
	{
		LibMath::Vector4 m_count;
		GPUDirectionalLight m_lights[MAX_DIR_LIGHTS];
	};

	struct alignas(16) GPUSpotLight
	{
		LibMath::Vector4 m_position;
		LibMath::Vector4 m_direction;
		LibMath::Vector4 m_color;
		LibMath::Vector4 m_param; // x = intensity, y = shadowIdx, z = innerCutoff, w = outerCutoff
	};

	struct alignas(16) GPUSpotLightBuffer
	{
		LibMath::Vector4 m_count;
		GPUSpotLight m_lights[MAX_SPOT_LIGHTS];
	};
}

#endif // !LIGHT


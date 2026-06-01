#ifndef RENDER_PASS_INFO
#define RENDER_PASS_INFO

#include "ResourceHandle.h"

class Shader;
namespace Apex::Rendering
{
	class Camera;
	class IRHI;

	enum Flags : uint8_t
	{
		ShowCollider = 1 << 0,
		ShadowPass = 1 << 1,
		IgnoreMaterial = 1 << 2,
		UseIdAsColor = 1 << 3,
		RenderSkybox = 1 << 4,
		PerformCompute = 1 << 5,
	};

	struct RenderPassInfo
	{
		Resources::ResourceHandle<Shader> m_shader = nullptr;
		Camera* m_camera = nullptr;
		IRHI* m_rhi = nullptr;
		float m_aspectRatio = 16.f / 9.f;
		RHIFrameBufferHandle m_target = RHI_INVALID;
		unsigned m_width = 1280;
		unsigned m_height = 720;
		uint8_t m_flags = RenderSkybox | PerformCompute;
	};
}

#endif // !RENDER_PASS_INFO
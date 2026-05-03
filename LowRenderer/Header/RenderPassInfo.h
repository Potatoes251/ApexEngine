#ifndef RENDER_PASS_INFO
#define RENDER_PASS_INFO

#include "ResourceHandle.h"
#include "RHI.h"

class Shader;
namespace Apex::Rendering
{
	class Camera;
	class IRHI;

	enum Flags : uint8_t
	{
		ShowCollider = 1 << 0,
		ShadowPass = 1 << 1,
		IgnoreModelShader = 1 << 2,
		UseIdAsColor = 1 << 3,
		RenderSkybox = 1 << 4,
	};

	struct RenderPassInfo
	{
		Resources::ResourceHandle<Shader> m_shader = Resources::ResourceHandle<Shader>(nullptr);
		Camera* m_camera = nullptr;
		IRHI* m_rhi = nullptr;
		float m_aspectRatio = 16.f / 9.f;
		RHIFrameBufferHandle m_target = RHI_INVALID;
		unsigned m_width = 1280;
		unsigned m_height = 720;
		int m_id = -1;
		uint8_t m_flags = RenderSkybox;
	};
}

#endif // !RENDER_PASS_INFO
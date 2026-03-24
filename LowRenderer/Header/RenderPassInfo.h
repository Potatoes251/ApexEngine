#ifndef RENDER_PASS_INFO
#define RENDER_PASS_INFO

#include "ResourceHandle.h"

class Shader;
namespace Apex::Rendering
{
	class Camera;
	struct RenderPassInfo
	{
		Resources::ResourceHandle<Shader> m_shader = Resources::ResourceHandle<Shader>(nullptr);
		Camera* m_camera = nullptr;
		float m_aspectRatio = 16.f / 9.f;
		bool m_useModelShader = true;
		bool m_useIdAsColor = false;
		bool m_showCollider = false;
		int m_id = -1;
	};
}

#endif // !RENDER_PASS_INFO
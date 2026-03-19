#ifndef SKYBOX
#define SKYBOX

#include "RHI.h"

#include "Cubemap.h"
#include "Mesh.h"
#include "Shader.h"
#include "ResourceHandle.h"

namespace Apex::Rendering
{
	class Skybox
	{
	public:
		Skybox(Resources::ResourceHandle<Cubemap> cubemap, Resources::ResourceHandle<Mesh> mesh, 
			Resources::ResourceHandle<Shader> shader, IRHI* rhi);
		Skybox(Skybox const&) = delete;
		Skybox& operator=(Skybox const&) = delete;

		void Render(LibMath::Matrix4 viewProj);
	private:
		Resources::ResourceHandle<Cubemap> m_cubemap;
		Resources::ResourceHandle<Mesh> m_mesh;
		Resources::ResourceHandle<Shader> m_shader;
		IRHI* m_rhi;
	};
}

#endif // !SKYBOX


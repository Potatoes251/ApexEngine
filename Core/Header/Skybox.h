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
			Resources::ResourceHandle<Shader> shader);
		Skybox(Skybox const&);
		Skybox& operator=(Skybox const&);

		void Render(LibMath::Matrix4 viewProj);
	private:
		Resources::ResourceHandle<Cubemap> m_cubemap;
		Resources::ResourceHandle<Mesh> m_mesh;
		Resources::ResourceHandle<Shader> m_shader;
	};
}

#endif // !SKYBOX


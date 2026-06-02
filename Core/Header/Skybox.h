#ifndef SKYBOX
#define SKYBOX

#include "ResourceHandle.h"

class Shader;
class Model;
class Cubemap;

namespace Apex::Rendering
{
	class Skybox
	{
	public:
		Skybox(Resources::ResourceHandle<Cubemap> cubemap, Resources::ResourceHandle<Model> model, 
			Resources::ResourceHandle<Shader> shader);
		Skybox(Skybox const&);
		Skybox& operator=(Skybox const&);

		void Render(LibMath::Matrix4 viewProj);
	private:
		Resources::ResourceHandle<Cubemap> m_cubemap;
		Resources::ResourceHandle<Model> m_model;
		Resources::ResourceHandle<Shader> m_shader;
	};
}

#endif // !SKYBOX


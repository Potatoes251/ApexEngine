#ifndef CUBE_RENDERER
#define CUBE_RENDERER

#include "RenderPassInfo.h"
#include "RHI.h"

#include "LibMath/Matrix/Matrix4.h"
#include "LibMath/Vector/Vector3.h"

#include "ResourceHandle.h"

#include "Object.h"

#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"

namespace Apex::Rendering
{
    class MeshRenderer : public Apex::Component
    {
    public:
        void SetMesh(Apex::Resources::ResourceHandle<Mesh>& mesh);
        void SetTexture(Apex::Resources::ResourceHandle<Texture>& shader);
        void SetShader(Apex::Resources::ResourceHandle<Shader>& texture);

        Apex::Resources::ResourceHandle<Mesh>& GetMesh();
        Apex::Resources::ResourceHandle<Texture>& GetTexture();
        Apex::Resources::ResourceHandle<Shader>& GetShader();

        void Render(RenderPassInfo passInfo);

        void Serialize(std::ostream& out) const override;

    private:
        Apex::Resources::ResourceHandle<Mesh> m_mesh;
        Apex::Resources::ResourceHandle<Shader> m_shader;
        Apex::Resources::ResourceHandle<Texture> m_texture;
    };
}

#endif
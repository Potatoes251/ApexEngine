#ifndef CUBE_RENDERER
#define CUBE_RENDERER

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
    private:
        Apex::Resources::ResourceHandle<Mesh> m_mesh;
        Apex::Resources::ResourceHandle<Shader> m_shader;
        Apex::Resources::ResourceHandle<Texture> m_texture;

    public:
        void SetMesh(Apex::Resources::ResourceHandle<Mesh>& mesh);
        void SetTexture(Apex::Resources::ResourceHandle<Texture>& shader);
        void SetShader(Apex::Resources::ResourceHandle<Shader>& texture);

        void Render(const LibMath::Matrix4& view,
            const LibMath::Matrix4& projection,
            const LibMath::Vector3& cameraPos);
    };
}

#endif
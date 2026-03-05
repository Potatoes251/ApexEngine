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
    struct MeshRenderer 
    {
        Apex::Resources::ResourceHandle<Mesh> m_mesh;
        Apex::Resources::ResourceHandle<Shader> m_shader;
        Apex::Resources::ResourceHandle<Texture> m_texture;

        std::shared_ptr<Apex::Data::Object> object = std::make_shared<Apex::Data::Object>();
    };
}

#endif
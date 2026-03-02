#ifndef CUBE_RENDERER
#define CUBE_RENDERER

#include "RHI.h"

#include "LibMath/Matrix/Matrix4.h"
#include "LibMath/Vector/Vector3.h"

#include "ResourceHandle.h"

#include "Object.h"

namespace Apex::Rendering
{
    struct MeshRenderer 
    {
        Apex::Resources::ResourceHandle m_mesh;
        Apex::Resources::ResourceHandle m_shader;
        Apex::Resources::ResourceHandle m_texture;

        std::shared_ptr<Apex::Data::Object> object = std::make_shared<Apex::Data::Object>();
    };
}

#endif
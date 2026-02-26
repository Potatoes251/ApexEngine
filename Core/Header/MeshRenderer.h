#ifndef CUBE_RENDERER
#define CUBE_RENDERER

#include "RHI.h"

#include "LibMath/Matrix/Matrix4.h"
#include "LibMath/Vector/Vector3.h"

#include "ResourceHandle.h"

namespace Apex::Rendering
{
    struct MeshRenderer 
    {
        Apex::Resources::ResourceHandle m_mesh;
        Apex::Resources::ResourceHandle m_shader;
        Apex::Resources::ResourceHandle m_texture;

        float  m_rotationSpeed = 20.f;          // degrees/s around Y
    };
}

#endif
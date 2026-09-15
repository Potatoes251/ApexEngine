#ifndef VERTEX
#define VERTEX

// ============================================================
// Vertex.h - Plain vertex layout shared between the RHI,
// resource system, physics, and any other code that needs
// the vertex format without pulling in the full RHI interface.
// ============================================================

#include "LibMath/Vector/Vector4.h"

namespace Apex::Rendering
{
    // uv is packed as w values of position and normal
    struct Vertex
    {
        LibMath::Vector4 m_position;    // w = u
        LibMath::Vector4 m_normal;      // w = v

        int		         m_boneIDs[4] = { -1, -1, -1, -1 };
        float	         m_weights[4] = { 0,0,0,0 };
    };

} // namespace Apex::Rendering

#endif
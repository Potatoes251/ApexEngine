#ifndef VERTEX
#define VERTEX

// ============================================================
// Vertex.h — Plain vertex layout shared between the RHI,
// resource system, physics, and any other code that needs
// the vertex format without pulling in the full RHI interface.
// ============================================================
#include "LibMath/Vector/Vector3.h"
#include "LibMath/Vector/Vector2.h"

namespace Apex::Rendering
{
    struct Vertex
    {
        LibMath::Vector3 m_position;
        LibMath::Vector3 m_normal;
        LibMath::Vector2 m_uv;

        int		         m_boneIDs[4] = { -1, -1, -1, -1 };
        float	         m_weights[4] = { 0,0,0,0 };
    };

} // namespace Apex::Rendering

#endif
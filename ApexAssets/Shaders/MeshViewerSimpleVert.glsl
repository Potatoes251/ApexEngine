#version 330 core

// Simple vertex shader for the MeshViewer.
// Used by wireframe, grid, bounding box, and pivot overlays.
// Only needs position — no normal or UV.

layout(location = 0) in vec3 aPos;

uniform mat4 uMVP;

void main()
{
    gl_Position = uMVP * vec4(aPos, 1.0);
}

#version 330 core

// Flat fragment shader for the MeshViewer.
// Outputs a single solid colour set via four float uniforms.
// Used by: wireframe mode, grid, bounding box, pivot axis lines,
//          and the normals overlay lines.
//
// Four separate float uniforms are used instead of a vec4 because
// the engine's IRHI only exposes SetUniformFloat / SetUniformVec3.

out vec4 FragColor;

uniform float uColor_r;
uniform float uColor_g;
uniform float uColor_b;
uniform float uColor_a;

void main()
{
    FragColor = vec4(uColor_r, uColor_g, uColor_b, uColor_a);
}

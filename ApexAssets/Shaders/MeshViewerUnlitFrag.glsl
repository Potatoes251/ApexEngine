#version 330 core

// Unlit fragment shader for the MeshViewer.
// Outputs the base colour directly, no lighting calculation.

in  vec3 vNormal;
in  vec3 vWorldPos;
in  vec2 vUV;

out vec4 FragColor;

uniform vec3  uBaseColor;
uniform float uExposure;

void main()
{
    FragColor = vec4(uBaseColor * uExposure, 1.0);
}

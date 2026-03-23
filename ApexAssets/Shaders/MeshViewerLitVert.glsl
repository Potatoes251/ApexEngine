#version 330 core

// Lit vertex shader for the MeshViewer.
// Passes world-space position, transformed normal, and UV to the fragment shader.

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

uniform mat4 uMVP;
uniform mat4 uModel;

out vec3 vNormal;
out vec3 vWorldPos;
out vec2 vUV;

void main()
{
    vec4 world  = uModel * vec4(aPos, 1.0);
    vWorldPos   = world.xyz;
    vNormal     = mat3(transpose(inverse(uModel))) * aNormal;
    vUV         = aUV;
    gl_Position = uMVP * vec4(aPos, 1.0);
}

#version 330 core

// Lit vertex shader for the MeshViewer.
// Passes world-space position, transformed normal, and UV to the fragment shader.

layout(location = 0) in vec4 aPos;
layout(location = 1) in vec4 aNormal;

uniform mat4 uMVP;
uniform mat4 uModel;

out vec3 vNormal;
out vec3 vWorldPos;
out vec2 vUV;

void main()
{
    vec4 world  = uModel * vec4(aPos.xyz, 1.0);
    vWorldPos   = world.xyz;
    vNormal     = mat3(transpose(inverse(uModel))) * aNormal.xyz;
    vUV         = vec2(aPos.w, aNormal.w);
    gl_Position = uMVP * vec4(aPos.xyz, 1.0);
}

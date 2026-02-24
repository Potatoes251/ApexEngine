#version 330 core

// ── Inputs ────────────────────────────────────────────────────
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

// ── Uniforms ──────────────────────────────────────────────────
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

// ── Outputs to fragment shader ────────────────────────────────
out vec3 vFragPos;
out vec3 vNormal;
out vec2 vTexCoord;

void main() 
{
    vec4 worldPos  = uModel * vec4(aPosition, 1.0);
    vFragPos       = worldPos.xyz;
    // Normal matrix: transpose of inverse of upper-left 3x3 of model
    vNormal        = mat3(transpose(inverse(uModel))) * aNormal;
    vTexCoord      = aTexCoord;
    gl_Position    = uProjection * uView * worldPos;
}
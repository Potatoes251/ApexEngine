#version 420 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uModel;
uniform mat4 uViewProj;
uniform mat4 lightSpaceMatrix[10];

out vec3 vFragPos;
out vec3 vNormal;
out vec2 vTexCoord;
out vec4 FragPosLightSpace[10];

struct DirLight
{
    vec4 dir;
    vec4 color;
    vec4 param; // x = intensity, y = shadowIdx
};

struct SpotLight
{
    vec4 pos;
    vec4 dir;
    vec4 color;
    vec4 param; // x = intensity, y = shadowIdx, z = innerCutoff, w = outerCutoff
};

layout(std140, binding = 1) uniform DirLightBuffer
{
    vec4 DirCount; // x = count
    DirLight DirLights[10];
};

layout(std140, binding = 2) uniform SpotLightBuffer
{
    vec4 SpotCount; // x = count
    SpotLight SpotLights[10];
};

void main() 
{
    vec4 worldPos  = uModel * vec4(aPosition, 1.0);
    vFragPos       = worldPos.xyz;
    // Normal matrix: transpose of inverse of upper-left 3x3 of model
    vNormal        = mat3(transpose(inverse(uModel))) * aNormal;
    vTexCoord      = aTexCoord;
    gl_Position    = uViewProj * worldPos;

    int count = int(DirCount.x + SpotCount.x);
    for (int i = 0; i < count; ++i)
	{
		FragPosLightSpace[i] = lightSpaceMatrix[i] * vec4(vFragPos, 1.0);
	}
}
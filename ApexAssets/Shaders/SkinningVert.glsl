#version 330 core

#define MAX_BONES 100
#define MAX_BONE_INFLUENCE 4

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in ivec4 aBoneIDs;
layout (location = 4) in vec4 aWeights;

uniform mat4 u_Bones[MAX_BONES];

uniform mat4 uModel;
uniform mat4 uViewProj;

out vec2 TexCoord;

void main()
{

    mat4 skinMatrix =
          aWeights[0] * u_Bones[aBoneIDs[0]]
        + aWeights[1] * u_Bones[aBoneIDs[1]]
        + aWeights[2] * u_Bones[aBoneIDs[2]]
        + aWeights[3] * u_Bones[aBoneIDs[3]];

    vec4 skinnedPosition = skinMatrix * vec4(aPosition, 1.0);

    gl_Position = uViewProj * uModel * skinnedPosition;

    TexCoord = aTexCoord;
}
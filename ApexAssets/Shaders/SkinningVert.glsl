#version 330 core

#define MAX_BONES 200
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
    mat4 skinMatrix = mat4(0.0);
    
    for (int i = 0; i < 4; ++i)
    {
        int id = aBoneIDs[i];
        float weight = aWeights[i];
    
        if (id >= 0 && weight > 0.0)
        {
            skinMatrix += weight * u_Bones[id];
        }
    }

    vec4 skinnedPosition = skinMatrix * vec4(aPosition, 1.0);

    gl_Position = uViewProj * uModel * skinnedPosition;

    TexCoord = aTexCoord;
}
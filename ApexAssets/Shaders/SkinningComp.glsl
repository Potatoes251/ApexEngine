#version 430 core

layout(local_size_x = 64) in;

#define MAX_BONES 200
#define MAX_BONE_INFLUENCE 4

struct Vertex {
    vec4 position;
    vec4 normal;
    ivec4 boneIDs;
    vec4 weights;
};

struct SkinnedVertex {
    vec4 position;
    vec4 normal;
    ivec4 boneIDs;
    vec4 weights;
};

layout(std430, binding = 0) readonly buffer InVertices {
    Vertex vertices[];
};

layout(std430, binding = 1) writeonly buffer OutVertices {
    SkinnedVertex skinned[];
};

uniform mat4 u_Bones[MAX_BONES];


void main()
{
    uint id = gl_GlobalInvocationID.x;

    Vertex v = vertices[id];

    vec4 pos = vec4(0.0);
    vec3 norm = vec3(0.0);

    mat4 skinMatrix = mat4(0.0);
    
    // Skinning
    for (int i = 0; i < 4; i++)
    {
        int boneID = v.boneIDs[i];
        float w = v.weights[i];

        if (w > 0.0)
        {
            mat4 bone = u_Bones[boneID];

            pos += bone * vec4(v.position.xyz, 1.0) * w;
            norm += mat3(bone) * v.normal.xyz * w;
        }
    }

    skinned[id].position = vec4(pos.xyz, v.position.w);
    skinned[id].normal = vec4(normalize(norm), v.normal.w);
}
#version 430 core

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

#define MAX_BONES 60
#define MAX_BONE_INFLUENCE 4

struct Vertex {
    vec4 position;
    vec4 normal;
    ivec4 boneIDs;
    vec4 weights;
};

layout(std430, binding = 0) readonly buffer InVertices {
    restrict Vertex vertices[];
};

layout(std430, binding = 1) writeonly buffer OutVertices {
    restrict Vertex skinned[];
};

uniform mat4 u_Bones[MAX_BONES];

void main()
{
    uint id = gl_GlobalInvocationID.x;
    uint totalVerts = uint(vertices.length());
    if (id >= totalVerts) return;

    Vertex vert = vertices[id];

    vec4 pos = vec4(0.0);
    vec3 norm = vec3(0.0);

    mat4 skinMatrix = mat4(0.0);
    
    // Skinning
    for (int i = 0; i < 4; i++)
    {
        int boneID = vert.boneIDs[i];
        float w = vert.weights[i];

        if (w > 0.0 && boneID >= 0 && boneID < MAX_BONES)
        {
            uint index = uint(boneID);
            mat4 bone = u_Bones[index];

            pos += bone * vec4(vert.position.xyz, 1.0) * w;
            norm += mat3(bone) * vert.normal.xyz * w;
        }
    }

    float u = vert.position.w;
    float v = vert.normal.w;

    Vertex out_v;
    out_v.position = vec4(pos.xyz, u);
    out_v.normal   = vec4(length(norm) > 0.0 ? normalize(norm) : vert.normal.xyz, v);
    out_v.boneIDs  = vert.boneIDs;
    out_v.weights  = vert.weights;
    skinned[id] = out_v;
}
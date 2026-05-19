#version 430 core

layout(local_size_x = 64) in;

struct Vertex 
{
    vec4 position;
    vec4 normal;
    ivec4 boneIDs;
    vec4 weights;
};

layout(std430, binding = 0) readonly buffer InVertices 
{
    Vertex vertices[];
};

layout(std430, binding = 1) writeonly buffer OutVertices 
{
    Vertex displaced[];
};

uniform float uTime;

float getDisplacement(vec2 uv) 
{
    float speed = 2;
    vec2  center = uv - 0.5;
    float dist   = length(center);
    float bulge  = exp(-dist * dist * 6.0) * sin(dist * 50.0 - uTime * speed) * 0.05;

    // Ripple also standing, slightly different frequency for variation
    float ripple = sin(dist * 18.0 - uTime * speed * 1.3) * 0.025;

    float pinch  = smoothstep(0.5, 0.35, dist);
    return (bulge + ripple) * pinch;
}

void main()
{
    uint index = gl_GlobalInvocationID.x;

    uint totalVerts = uint(vertices.length());
    if (index >= totalVerts) return;

    Vertex vin = vertices[index];

    if (uTime == 0)
    {
        displaced[index] = vin;
        return;
    }

    vec2 uv = vec2(vin.position.w, vin.normal.w);

    float eps = 1.0 / 128.0;
    float d   = getDisplacement(uv);
    float dX  = getDisplacement(uv + vec2(eps, 0.0));
    float dY  = getDisplacement(uv + vec2(0.0, eps));

    // Displace position along Z
    Vertex vout;
    vout.position   = vin.position;
    vout.position.z += d;

    // Recompute normal from gradient (cross product of tangents)
    // Tangent along X: neighbor moves right by eps*2, Z changes by dX-d
    // Tangent along Y: neighbor moves up   by eps*2, Z changes by dY-d
    vec3 tangentX = normalize(vec3(eps * 2.0, 0.0,     dX - d));
    vec3 tangentY = normalize(vec3(0.0,       eps * 2.0, dY - d));
    vec3 newNormal = cross(tangentX, tangentY); // points toward +Z when flat

    vout.normal   = vec4(newNormal, uv.y);

    vout.boneIDs  = vin.boneIDs;
    vout.weights  = vin.weights;

    displaced[index] = vout;
}
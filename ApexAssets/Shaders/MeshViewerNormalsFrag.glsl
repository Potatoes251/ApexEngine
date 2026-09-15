#version 330 core

// Normals fragment shader for the MeshViewer.
// Visualises the surface normal direction as an RGB colour.
// Normal components are remapped from [-1, 1] to [0, 1] for display:
//   +X (right)    = red
//   +Y (up)       = green
//   +Z (forward)  = blue

in  vec3 vNormal;
in  vec3 vWorldPos;
in  vec2 vUV;

out vec4 FragColor;

void main()
{
    vec3 n    = normalize(vNormal);
    vec3 col  = n * 0.5 + 0.5;   // remap [-1,1] -> [0,1]
    FragColor = vec4(col, 1.0);
}

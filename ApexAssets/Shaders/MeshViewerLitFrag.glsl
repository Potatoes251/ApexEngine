#version 330 core

// Lit fragment shader for the MeshViewer.
// Simple Blinn-Phong with ambient + diffuse + specular.

in vec3 vNormal;
in vec3 vWorldPos;
in vec2 vUV;

out vec4 FragColor;

uniform vec3  uLightDir;   // world-space directional light direction (unit vector)
uniform vec3  uCamPos;     // world-space camera position
uniform float uExposure;   // EV multiplier
uniform vec3  uBaseColor;  // mesh base colour

void main()
{
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightDir);

    float diff    = max(dot(N, L), 0.0);
    vec3  H       = normalize(L + normalize(uCamPos - vWorldPos));
    float spec    = pow(max(dot(N, H), 0.0), 64.0);

    vec3 ambient  = uBaseColor * 0.15;
    vec3 diffuse  = uBaseColor * diff * 0.75;
    vec3 specular = vec3(1.0) * spec * 0.25;

    vec3 color = (ambient + diffuse + specular) * uExposure;
    FragColor  = vec4(color, 1.0);
}

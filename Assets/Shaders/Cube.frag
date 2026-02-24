#version 330 core

// ── Inputs from vertex shader ─────────────────────────────────
in vec3 vFragPos;
in vec3 vNormal;
in vec2 vTexCoord;

// ── Uniforms ──────────────────────────────────────────────────
uniform sampler2D uTexture;
uniform int       uUseTexture;

uniform vec3 uLightPos;
uniform vec3 uLightColor;
uniform vec3 uViewPos;

// ── Output ────────────────────────────────────────────────────
out vec4 FragColor;

void main() 
{
    // Base color: texture or solid color
    vec3 baseColor = (uUseTexture == 1)
        ? texture(uTexture, vTexCoord).rgb
        : vec3(0.72, 0.45, 0.20);   // warm orange fallback

    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightPos - vFragPos);
    vec3 V = normalize(uViewPos  - vFragPos);
    vec3 R = reflect(-L, N);

    // Phong model
    float ambient   = 0.15;
    float diffuse   = max(dot(N, L), 0.0);
    float specular  = pow(max(dot(V, R), 0.0), 32.0) * 0.5;

    vec3 color = baseColor * uLightColor * (ambient + diffuse) +
                 uLightColor * specular;

    FragColor = vec4(color, 1.0);
}
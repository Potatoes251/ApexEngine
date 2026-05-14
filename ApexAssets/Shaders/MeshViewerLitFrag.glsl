#version 330 core

in vec3 vNormal;
in vec3 vWorldPos;
in vec2 vUV;

out vec4 FragColor;

uniform vec3  uLightDir;
uniform vec3  uCamPos;
uniform float uExposure;
uniform vec3  uBaseColor;
uniform vec3  uTint;
uniform int   uUseTexture;

uniform sampler2D uAlbedoTex;

void main()
{
    // Base colour: texture * tint, or baseColor * tint
    vec3 albedo;
    if (uUseTexture == 1)
        albedo = texture(uAlbedoTex, vUV).rgb * uTint;
    else
    {
        albedo = uBaseColor;
    }

    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightDir);
    vec3 V = normalize(uCamPos - vWorldPos);
    vec3 H = normalize(L + V);

    float diff    = max(dot(N, L), 0.0);
    float spec    = pow(max(dot(N, H), 0.0), 64.0);

    vec3 ambient  = albedo * 0.18;
    vec3 diffuse  = albedo * diff * 0.78;
    vec3 specular = vec3(0.4) * spec;

    vec3 color = (ambient + diffuse + specular) * uExposure;
    FragColor  = vec4(clamp(color, 0.0, 1.0), 1.0);
}

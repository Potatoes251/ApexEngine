#version 420 core

in vec3 vFragPos;
in vec3 vNormal;
in vec2 vTexCoord;

layout(binding = 0) uniform sampler2D uTexture;

uniform float time;

void main()
{
	vec2 uv = vTexCoord;

    // Ripple distortion
    float strength = 0.03;
    uv.x += sin(uv.y * 10.0 + time * 2.0) * strength;
    uv.y += sin(uv.x * 10.0 + time * 2.0) * strength;

    gl_FragColor = texture2D(uTexture, uv);
}
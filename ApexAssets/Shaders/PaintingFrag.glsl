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
    float speed = 2.0;
    float frequency = 10.0;
    uv.x += sin(uv.y * frequency + time * speed) * strength;
    uv.y += sin(uv.x * frequency + time * speed) * strength;

    gl_FragColor = texture2D(uTexture, uv);
}
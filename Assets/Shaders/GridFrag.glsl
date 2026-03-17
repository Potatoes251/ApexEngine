#version 330 core

in vec3 vWorldPos;
out vec4 FragColor;

uniform vec3 uCameraPos;

// Grid settings
uniform float uMajorScale = 1.0;   // size of major grid
uniform float uMinorScale = 0.1;   // size of subdivisions

uniform vec4 uColorMajor = vec4(0.7, 0.7, 0.7, 1.0);
uniform vec4 uColorMinor = vec4(0.3, 0.3, 0.3, 1.0);
uniform vec4 uColorAxisX = vec4(1.0, 0.2, 0.2, 1.0);
uniform vec4 uColorAxisZ = vec4(0.2, 0.4, 1.0, 1.0);

float gridLine(float coord, float scale)
{
    float line = abs(fract(coord / scale - 0.5) - 0.5) / fwidth(coord / scale);
    return 1.0 - clamp(line, 0.0, 1.0);
}

void main()
{
    vec2 coord = vWorldPos.xz + uCameraPos.xz;

    // Minor grid
    float minor = max(gridLine(coord.x, uMinorScale), gridLine(coord.y, uMinorScale));

    // Major grid
    float major = max(gridLine(coord.x, uMajorScale), gridLine(coord.y, uMajorScale));

    // Axis highlight
    float axisX = 1.0 - clamp(abs(coord.y) / fwidth(coord.y), 0.0, 1.0);
    float axisZ = 1.0 - clamp(abs(coord.x) / fwidth(coord.x), 0.0, 1.0);

    vec4 color = vec4(0.0);

    // Blend grids
    color += uColorMinor * minor * 0.5;
    color += uColorMajor * major;

    // Axes override
    color = mix(color, uColorAxisX, axisX);
    color = mix(color, uColorAxisZ, axisZ);

    // Distance fade
    float dist = length(vWorldPos - uCameraPos);
    float fade = exp(-dist * 0.02);
    color.a *= fade;

    if (color.a <= 0.01)
        discard;

    FragColor = color;
}
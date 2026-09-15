#version 330 core

uniform mat4 uInvViewProj;
uniform mat4 uViewProj;
uniform vec3 uCameraPos;
uniform float near = 0.01;
uniform float far = 100;
uniform float gridScale = 10;
uniform float minorThickness = 0.01;
uniform float majorThickness = 0.001;
uniform float fadeStart = 20.0; // distance where fading begins
uniform float fadeEnd   = 100.0; // distance where grid fully disappears
uniform vec3 minorColor = vec3(0.1);
uniform vec3 majorColor = vec3(0.5);

in vec2 vUV;
out vec4 outColor;

vec3 GetWorldPos(vec2 uv, float z)
{
    vec4 clip = vec4(uv * 2.0 - 1.0, z, 1.0);
    vec4 world = uInvViewProj * clip;
    return world.xyz / world.w;
}

void main()
{
    vec3 nearPos = GetWorldPos(vUV, -1.0);
    vec3 farPos  = GetWorldPos(vUV,  1.0);

    vec3 rayDir = normalize(farPos - nearPos);
    vec3 rayOrigin = nearPos;

    if (abs(rayDir.y) < 0.0001) discard;

    float t = -rayOrigin.y / rayDir.y;
    if (t < 0.0) discard;

    vec3 hit = rayOrigin + rayDir * t;

    vec4 clip = uViewProj * vec4(hit, 1.0);
    float ndcDepth = clip.z / clip.w;
    gl_FragDepth = ndcDepth * 0.5 + 0.5; 

    vec2 coord = hit.xz;

    float minorGridScale = gridScale * .1;
    vec2 grid = abs(fract(coord * minorGridScale - 0.5) - 0.5);
    float line = min(grid.x, grid.y);
    float aa = fwidth(line);   
    float minor = 1.0 - smoothstep(0, minorThickness + aa, line);
    minor = min(minor, 0.25);

    vec2 majorCoord = coord;
    vec2 majorGrid = abs(fract(majorCoord / gridScale - 0.5) - 0.5);
    float majorLine = min(majorGrid.x, majorGrid.y);
    aa = fwidth(majorLine);   
    float major = 1.0 - smoothstep(0, majorThickness + aa, majorLine);


    float axisX = smoothstep(0, majorThickness * 10, abs(coord.y));
    float axisZ = smoothstep(0, majorThickness * 10, abs(coord.x));

    float dist = length(hit - uCameraPos);
    float fade = 1.0 - smoothstep(fadeStart, fadeEnd, dist); // invert

    vec3 color = minorColor * minor + majorColor * major;

    color = mix(vec3(1,0,0), color, axisX); // X axis
    color = mix(vec3(0,0,1), color, axisZ); // Z axis

    color *= fade;

    if (color.r <= 0.01 && color.g <= 0.01 && color.b <= 0.01)
        discard;

    outColor = vec4(color, fade);
}
#version 330 core
in vec4 FragPos;

uniform vec3 lightPos;
uniform float farPlane;

#define BIAS 0.01

void main()
{
    float lightDistance = length(FragPos.xyz - lightPos);
    
    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / farPlane;
    
    // write this as modified depth
    gl_FragDepth = lightDistance;
    gl_FragDepth += gl_FrontFacing ? BIAS : 0.0;
}
#version 330 core

#define BIAS 0.0002

void main()
{             
    gl_FragDepth = gl_FragCoord.z;
    gl_FragDepth += gl_FrontFacing ? 0.0 : BIAS;
}  
#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 aTexCoord;


uniform mat4 u_MVP;
uniform mat3 u_NormalMat;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

void main()
{
	gl_Position = u_MVP * vec4(position, 1.0);
	FragPos = position;
	Normal = u_NormalMat * normal;
	
    TexCoord = aTexCoord;
}
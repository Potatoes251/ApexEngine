#version 330 core

in vec2 TexCoord;      // from vertex shader
out vec4 FragColor;    // final color output

uniform sampler2D uTexture;  // diffuse texture

void main()
{
    // sample the texture at the given UV
    vec4 texColor = texture(uTexture, TexCoord);

    // output the final color
    FragColor = texColor;
}
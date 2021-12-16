#version 330 core
precision mediump float;
in vec2 textureCoord;
uniform sampler2D sampler;
out vec4 fragColor;
void main()
{
   fragColor=texture(sampler, textureCoord);
}

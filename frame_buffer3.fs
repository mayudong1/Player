#version 330 core
precision mediump float;
in vec2 textureCoord;
uniform sampler2D sampler;
out vec4 fragColor;
void main()
{
   vec4 color = texture(sampler, textureCoord);
   color.r = color.r - 0.1;
   color.g = color.g - 0.1;
   color.b = color.b - 0.1;
   fragColor=color;
}

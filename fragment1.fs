#version 330 core

in vec2 TexCoord;

uniform sampler2D tex_y;
uniform sampler2D tex_u;
uniform sampler2D tex_v;

uniform mat3 colorMatrix;

out vec4 FragColor;
void main()
{
	vec3 yuv;
    vec3 rgb;    
    yuv.x = texture(tex_y, TexCoord).r - (16.0 / 256.0);
    yuv.y = texture(tex_u, TexCoord).r - 0.5;
    yuv.z = texture(tex_v, TexCoord).r - 0.5;

    rgb = colorMatrix * yuv;

	FragColor = vec4(rgb, 1);
}
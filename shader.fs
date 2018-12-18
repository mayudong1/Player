#version 330 core

in vec2 TexCoord;

uniform sampler2D tex_y;
uniform sampler2D tex_u;
uniform sampler2D tex_v;

out vec4 FragColor;
void main()
{
	vec3 yuv;
    vec3 rgb;    
    yuv.x = texture(tex_y, TexCoord).r;
    yuv.y = texture(tex_u, TexCoord).r - 0.5;
    yuv.z = texture(tex_v, TexCoord).r - 0.5;

    rgb = mat3( 1,       1,         1,
                0,       -0.39465,  2.03211,
                1.13983, -0.58060,  0) * yuv;

	FragColor = vec4(rgb, 1);
}
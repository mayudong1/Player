#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main()
{
	mat4 rotateMat = mat4(0, -1, 0, 0,
						  1, 0, 0, 0,
						  0, 0, 1, 0,
						  0, 0, 0, 1);//旋转90度
	gl_Position = rotateMat*vec4(aPos.x, aPos.y, aPos.z, 1.0);
	TexCoord = vec2(aTexCoord.x, 1-aTexCoord.y);
}
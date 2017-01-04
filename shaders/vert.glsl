#version 330

layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec3 vertexNorm;

uniform mat4 MVP;
uniform float scale;

out vec3 out_pos;
out vec3 out_norm;

void main()
{
	vec3 position = vertexPos;
	position.x *= scale;
	position.y *= scale;
	position.z *= scale;

	gl_Position = MVP * vec4(position,1.0);

    out_pos = vertexPos;
	out_norm = vertexNorm;

}

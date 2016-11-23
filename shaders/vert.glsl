#version 330

layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec3 vertexNorm;

smooth out vec3 norm;
out vec3 pos;

uniform mat4 M;
uniform mat4 V;
uniform mat4 MVP;

void main()
{
    gl_Position = MVP * vec4(vertexPos, 1.0);

    norm = vertexNorm;

}

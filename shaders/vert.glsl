#version 330

//in vec4 vertex;

//uniform mat4 MVP;

//void main( void )
//{
  //gl_Position = MVP * vertex;
//}

layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec3 vertexNorm;

out vec3 norm;
out vec3 pos;

uniform mat4 MV;
uniform mat3 N;
uniform mat4 MVP;

void main()
{
    norm = normalize(N * vertexNorm);
    pos  = vec3(MV * vec4(vertexPos, 1.0));

    gl_Position = MVP * vec4(vertexPos, 1.0);
}

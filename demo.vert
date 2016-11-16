#version 400

layout (location = 0) in vec3 vertCoords;
layout (location = 1) in vec3 normCoords;

flat out float lightIntensity;

uniform mat4 vertMatrix;
uniform mat4 normMatrix;


void main()
{
    gl_Position = vertMatrix * vec4(vertCoords, 1);
    vec4 lNorm = normMatrix * vec4(normCoords, 1);
    lightIntensity = abs(lNorm.z);
}

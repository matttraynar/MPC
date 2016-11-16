#version 400

flat in float lightIntensity;

layout (location = 0) out vec4 fragColour;

uniform vec3 fragmentColour;

void main()
{
    fragColour = vec4(fragmentColour * lightIntensity);
}

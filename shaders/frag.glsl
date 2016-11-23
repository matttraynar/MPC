#version 330

in vec3 norm;
in vec3 pos;

out vec4 fragColour;

uniform mat4 M;
uniform mat4 MVP;

void main()
{
    vec3  lightDir       = vec3(-10,-10,-10);
    vec3  lightCol       = vec3(0.0,1.0,0.0);
    float lightIntensity = 0.1;

    vec4 colour = vec4(0.9,0.9,0.9,1.0);

    float brightness = max(0.0, dot(normalize(norm),lightDir));


    fragColour = colour * vec4(lightCol * (lightIntensity + brightness), 1.0);
}



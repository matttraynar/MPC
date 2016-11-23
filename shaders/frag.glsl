#version 330

in vec3 norm;
in vec3 pos;

out vec4 fragColour;


void main()
{
    vec3 lightPos = vec3(10,10,10);
    vec3 colour = vec3(0.9,0.9,0.9);

    vec3 lDir = normalize(lightPos - pos);
    float ltoN = max(dot(norm, lDir), 0.0);

    vec3 shadeColour = clamp(colour * 0.4 + colour * 0.6 * ltoN, 0.0f, 1.0f);

    fragColour = vec4(colour,1.0f);
}



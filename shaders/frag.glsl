#version 330

//layout(location = 0, index = 0) out vec4 fragColor;

//void main( void )
//{
 //fragColor = vec4( 0.9, 0.9, 0.9, 1.0 );
//}

in vec3 norm;
in vec3 pos;

layout (location = 0) out vec4 fragColour;

struct Light
{
    vec4 position;
    vec3 strength;
};

uniform Light light;

void main()
{
    fragColour = vec4(0.9f,0.9f,0.9f,1.0f);
}



#version 330

in vec3 out_pos;
in vec3 out_norm;

uniform mat4 M;
uniform vec4 mCol;

out vec4 fragColour;

void main()
{
    vec3 lPos = vec3(6.0, 6.0, 0.0);
    vec4 lCol = vec4(0.8, 0.8, 0.9, 1.0);

    vec3 v_pos = vec4(M * vec4(out_pos,1.0)).xyz;

    vec3 ws_pos = lPos - v_pos;

    float brightness = clamp(dot(out_norm,normalize(ws_pos)),0.0,1.0);

    fragColour = vec4(brightness * lCol.rgb * mCol.rgb,1.0);

}

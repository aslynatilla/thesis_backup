#version 330 core

in vec3 frag_pos;
in vec3 normal;

out vec4 FragColor;

uniform vec3 light_position;
uniform float far_plane;

void main()
{
    vec3 light_to_fragment = frag_pos - light_position;
    float distance_from_light = length(light_to_fragment);
    FragColor = vec4(distance_from_light/far_plane, 0.0, 0.0, 1.0);
}
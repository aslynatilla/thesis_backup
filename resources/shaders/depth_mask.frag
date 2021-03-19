#version 330 core

in vec3 frag_pos;
in vec3 frag_normal;

out vec4 FragColor;

uniform vec3 light_position;
uniform float far_plane;

void main()
{
    vec3 light_to_fragment = frag_pos - light_position;
    float distance_from_light = length(light_to_fragment);
    float scaled_distance = distance_from_light/far_plane;
    FragColor = vec4(frag_normal, 1.0);
}
#version 430 core

in vec3 frag_pos;
in vec3 frag_normal;

out vec4 FragColor;

layout (location = 9) uniform vec3 light_position;
layout (location = 10) uniform float far_plane;

layout (location = 11) uniform float furthest_distance;

void main()
{
    vec3 light_to_fragment = frag_pos - light_position;
    float distance_from_light = length(light_to_fragment);

    float scaled_distance = distance_from_light / furthest_distance;
    float emitting_along_direction_l = 1.0;

    FragColor = vec4(distance_from_light, scaled_distance, emitting_along_direction_l, 1.0);
}
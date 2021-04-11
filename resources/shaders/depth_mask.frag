#version 430 core

in vec3 frag_pos;
in vec3 frag_normal;

out vec4 FragColor;

uniform vec3 light_position;
uniform float far_plane;

uniform float furthest_distance;

void main()
{
    vec3 light_to_fragment = frag_pos - light_position;
    float distance_from_light = length(light_to_fragment);

    //  furthest_distance might be swapped with far_plane
    //  so that scaling doesn't matter
    float inverted_scaled_distance = 1.0 - (distance_from_light / far_plane);

    FragColor = vec4(inverted_scaled_distance, inverted_scaled_distance, inverted_scaled_distance, 1.0);
}
#version 430 core

in vec3 frag_pos;
in vec3 frag_normal;

out vec4 FragColor;

uniform vec3 light_position;
uniform float far_plane;

layout(location = 0) uniform float furthest_distance;

void main()
{
    vec3 light_to_fragment = frag_pos - light_position;
    float distance_from_light = length(light_to_fragment);
    float inverted_scaled_distance = 1.0 - distance_from_light/furthest_distance;
    FragColor = vec4(inverted_scaled_distance, inverted_scaled_distance, inverted_scaled_distance, 1.0);
}
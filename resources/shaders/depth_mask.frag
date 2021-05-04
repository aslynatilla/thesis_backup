#version 430 core

in vec3 frag_pos;
in vec3 frag_normal;

out vec4 FragColor;

layout(std140, binding = 1) uniform Light{
    vec4 position;
    vec4 direction;
    float constant_attenuation;
    float linear_attenuation;
    float quadratic_attenuation;

    float intensity;
    vec4 color;
} scene_light;

layout(std140, binding = 3) uniform CommonData{
    float light_camera_far_plane;
    float distance_to_furthest_ies_vertex;
    bool is_using_ies_masking;
};

void main()
{
    vec3 light_to_fragment = frag_pos - scene_light.position.xyz;
    float distance_from_light = length(light_to_fragment);

    float scaled_distance = distance_from_light / distance_to_furthest_ies_vertex;
    float emitting_along_direction_l = 1.0;

    FragColor = vec4(distance_from_light, scaled_distance, emitting_along_direction_l, 1.0);
}
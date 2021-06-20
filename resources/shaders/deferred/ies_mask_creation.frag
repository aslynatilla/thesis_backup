#version 430 core

in vec4 fragment_position;
in vec4 light_space_fragment_position;
in vec3 fragment_normal;

layout(std140, binding = 2) uniform Light{
    vec4 position;
    vec4 direction;
    float constant_attenuation;
    float linear_attenuation;
    float quadratic_attenuation;
    float intensity;
    vec4 color;
} scene_light;

layout(std140, binding = 3) uniform CommonData{
    vec4 camera_position;
    float light_camera_far_plane;
    float shadow_threshold;
    float distance_to_furthest_ies_vertex;
};

layout (location = 0) out vec4 ies_mask;

void main(){
    vec3 light_to_fragment = fragment_position.xyz - scene_light.position.xyz;
    float distance_from_light = length(light_to_fragment);

    float scaled_distance = distance_from_light / distance_to_furthest_ies_vertex;
    float emitting_along_direction_l = 1.0;

    ies_mask = vec4(distance_from_light, scaled_distance, emitting_along_direction_l, 1.0);
}
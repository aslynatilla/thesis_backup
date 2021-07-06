#version 430 core
#define NUMBER_OF_LIGHTS 2

in vec4 fragment_position;
in vec4 light_space_fragment_position;
in vec3 fragment_normal;

layout(std140, binding = 2) uniform Lights{
    vec4 positions[NUMBER_OF_LIGHTS];
    vec4 directions[NUMBER_OF_LIGHTS];
    vec4 attenuations_and_intensities[NUMBER_OF_LIGHTS];    //constant, linear, quadratic attenuation and intensity
    vec4 colors[NUMBER_OF_LIGHTS];
} scene_lights;

layout(std140, binding = 3) uniform CommonData{
    vec4 camera_position;
    float shadow_threshold;
    float light_camera_far_planes[NUMBER_OF_LIGHTS];
    float distances_to_furthest_ies_vertex[NUMBER_OF_LIGHTS];
};

layout (location = 6) uniform int light_index;

layout (location = 0) out vec4 ies_mask;

void main(){
    vec3 light_to_fragment = fragment_position.xyz - scene_lights.positions[light_index].xyz;
    float distance_from_light = length(light_to_fragment);

    float scaled_distance = distance_from_light / distances_to_furthest_ies_vertex[light_index];
    float emitting_along_direction_l = 1.0;

    ies_mask = vec4(distance_from_light, scaled_distance, emitting_along_direction_l, 1.0);
}
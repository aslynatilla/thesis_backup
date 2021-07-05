#version 430 core
#define NUMBER_OF_LIGHTS 2

in vec4 fragment_position;
in vec4 light_space_fragment_position;
in vec3 fragment_normal;
in vec2 fragment_uv_coords;

layout (std140, binding = 1) uniform MaterialProperties{
    vec4 diffuse_color;
    float shininess;
};

layout(std140, binding = 2) uniform Light{
    vec4 position;
    vec4 direction;
    float constant_attenuation;
    float linear_attenuation;
    float quadratic_attenuation;
    float intensity;
    vec4 color;
} scene_lights[NUMBER_OF_LIGHTS];

layout(std140, binding = 3) uniform CommonData{
    vec4 camera_position;
    float shadow_threshold;
    float light_camera_far_planes[NUMBER_OF_LIGHTS];
    float distances_to_furthest_ies_vertex[NUMBER_OF_LIGHTS];
};

layout (location = 6) uniform samplerCube ies_masking_texture;
layout (location = 7) uniform sampler2D diffuse_texture;
layout (location = 8) uniform int light_index;

layout (location = 0) out vec4 fragment_world_coords;
layout (location = 1) out vec4 fragment_normals;
layout (location = 2) out vec4 fragment_fluxes;

void main(){
    vec3 light_to_fragment = fragment_position.xyz - scene_lights[light_index].position.xyz;
    float distance_to_light = length(light_to_fragment);
    vec3 l = normalize(light_to_fragment);

    gl_FragDepth = distance_to_light / light_camera_far_planes[light_index];

    fragment_world_coords = vec4(fragment_position.xyz, 1.0);

    fragment_normals = vec4(fragment_normal, 1.0);

    vec3 ies_mask_data = texture(ies_masking_texture, l).rgb;
    vec4 computed_flux = diffuse_color * scene_lights[light_index].color * scene_lights[light_index].intensity;
    float is_emitting_light_along_l = ies_mask_data.b;
    float intensity_modifier = ies_mask_data.g;

    //  Temporarily disabling this line allows to render without a IES mask
    vec4 tex_color = texture(diffuse_texture, fragment_uv_coords).rgba;
    computed_flux *= tex_color;
    fragment_fluxes = vec4(computed_flux.xyz * intensity_modifier * is_emitting_light_along_l, 1.0);
    //  fragment_fluxes = computed_flux;
}
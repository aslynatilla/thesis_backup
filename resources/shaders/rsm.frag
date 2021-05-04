#version 430 core

in vec4 frag_pos;
in vec3 frag_normal;
in vec4 light_space_frag_pos;

layout(std140, binding = 1) uniform Light{
    vec4 position;
    vec4 direction;

    float constant_attenuation;
    float linear_attenuation;
    float quadratic_attenuation;
} scene_light;

layout (location = 6) uniform vec4 diffuse_color;

layout (location = 7) uniform float far_plane;
layout (location = 8) uniform float light_intensity;
layout (location = 9) uniform vec4 light_color = vec4(1.0, 1.0, 1.0, 1.0);

layout (location = 10) uniform samplerCube ies_mask;
layout (location = 11) uniform bool is_masking;

layout (location = 0) out vec4 fragment_world_coordinates;
layout (location = 1) out vec4 fragment_normal;
layout (location = 2) out vec4 fragment_flux;

void main(){
    vec3 light_to_fragment = frag_pos.xyz - scene_light.position.xyz;
    float light_distance = length(light_to_fragment);
    vec3 l = normalize(light_to_fragment);
    gl_FragDepth = light_distance / far_plane;

    float attenuation_factor = 1.0/(scene_light.constant_attenuation +
        scene_light.linear_attenuation * light_distance +
        scene_light.quadratic_attenuation * light_distance * light_distance);

    fragment_world_coordinates = vec4(frag_pos.xyz, 1.0);
    fragment_normal = vec4(frag_normal, 1.0);

    vec3 mask_data = texture(ies_mask, l).rgb;
    vec4 computed_flux = vec4(diffuse_color.xyz, 1.0) * light_color;

    if(is_masking == false){
        fragment_flux = computed_flux;
    } else {
        float is_emitting_along_l = mask_data.b;
        float intensity_modifier = mask_data.g;
        fragment_flux = vec4(computed_flux.xyz * intensity_modifier * is_emitting_along_l, 1.0);
    }
}
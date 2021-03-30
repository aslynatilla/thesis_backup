#version 330 core

in vec4 frag_pos;
in vec3 frag_normal;
in vec4 light_space_frag_pos;


uniform vec4 diffuse_color;

struct Light{
    vec3 position;
    vec3 direction;
    float cutoff_angle;
    float outer_cutoff_angle;

    float constant_attenuation;
    float linear_attenuation;
    float quadratic_attenuation;
};

uniform Light scene_light;
uniform float far_plane;
uniform float light_intensity;

uniform sampler2D ies_masking;
uniform bool is_masking;

layout (location = 0) out vec4 fragment_world_coordinates;
layout (location = 1) out vec4 fragment_normal;
layout (location = 2) out vec4 fragment_flux;

void main(){
    vec3 light_to_fragment = frag_pos.xyz - scene_light.position;
    float light_distance = length(light_to_fragment);
    gl_FragDepth = light_distance / far_plane;


    float attenuation_factor = 1.0/(scene_light.constant_attenuation +
        scene_light.linear_attenuation * light_distance +
        scene_light.quadratic_attenuation * light_distance * light_distance);

    float angle_between_light_dir_and_light_to_frag = dot(scene_light.direction, normalize(light_to_fragment));
    float epsilon = scene_light.cutoff_angle - scene_light.outer_cutoff_angle;
    float spotlight_intensity = clamp((angle_between_light_dir_and_light_to_frag - scene_light.outer_cutoff_angle)
                                        / epsilon,
                                        0.0, 1.0);

    fragment_world_coordinates = vec4(frag_pos.xyz, 1.0);
    fragment_normal = vec4(frag_normal, 1.0);

    //  Dachsbacher says:
    //       For a uniform spotlight, this flux decreases with the cosine to
    //      the spot direction due to the decreasing solid angle.
    //      The reflected flux is then the flux through the pixel times
    //      the reflection coefficient of the surface. No distance attenuation
    //      or receiver cosine must be computed.
    //  ...so the following line should not be right. Therefore we delete "attenuation_factor"...
    //  fragment_flux = diffuse_color.xyz * attenuation_factor * spotlight_intensity * light_intensity;

    vec2 sampling_coords = light_space_frag_pos.xy/light_space_frag_pos.w * 0.5 + 0.5;
    float mask_component = texture(ies_masking, sampling_coords).r;

    vec4 computed_flux = vec4(diffuse_color.xyz * spotlight_intensity, 1.0);
    fragment_flux = is_masking ? vec4(computed_flux.xyz * mask_component, 1.0) : computed_flux;
}
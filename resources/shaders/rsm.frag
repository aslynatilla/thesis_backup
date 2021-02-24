#version 330 core

in vec3 frag_pos;
in vec3 frag_normal;

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

layout (location = 0) out vec3 fragment_world_coordinates;
layout (location = 1) out vec3 fragment_normal;
layout (location = 2) out vec3 fragment_flux;

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

    fragment_world_coordinates = frag_pos;
    fragment_normal = frag_normal;

    //  Dachsbacher says:
    //       For a uniform spotlight, this flux decreases with the cosine to
    //      the spot direction due to the decreasing solid angle.
    //      The reflected flux is then the flux through the pixel times
    //      the reflection coefficient of the surface. No distance attenuation
    //      or receiver cosine must be computed.
    //  ...so the following line should not be right. Therefore we delete "attenuation_factor"...
    //  fragment_flux = diffuse_color.xyz * attenuation_factor * spotlight_intensity * light_intensity;

    fragment_flux = diffuse_color.xyz * spotlight_intensity * light_intensity;
}
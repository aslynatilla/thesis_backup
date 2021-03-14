#version 330 core

in vec3 frag_pos;
in vec4 light_frag_pos;
in vec3 normal;

out vec4 FragColor;

uniform float opacity;
uniform float shininess;
uniform float refract_i;

uniform vec4 diffuse_color;
uniform vec4 ambient_color;
uniform vec4 specular_color;
uniform vec4 emissive_color;
uniform vec4 transparent_color;

uniform vec3 camera_position;
uniform vec3 light_position;

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

uniform sampler2D shadow_map;
uniform sampler2D position_map;
uniform sampler2D normal_map;
uniform sampler2D flux_map;
uniform sampler1D sample_array;

uniform int samples_number;

uniform float far_plane;

// TWEAKABLES
uniform float shadow_threshold;

uniform float max_radius;
uniform float indirect_intensity;
uniform float light_intensity;

uniform bool hide_direct_component;

float compute_shadow(float light_distance)
{
    //  Compute fragment position in light space
    vec3 fragment_light_space_coordinates = light_frag_pos.xyz / light_frag_pos.w;
    //  Move it in the unit square [0, 1] x [0, 1]
    fragment_light_space_coordinates = fragment_light_space_coordinates * 0.5 + 0.5;

    //  Sample the shadow_map and multiply it for the far plane distance, so you can compare it to the distance
    // of the fragment from the light
    float depth = texture(shadow_map, fragment_light_space_coordinates.xy).r;
    depth *= far_plane;

    if (light_distance < depth + shadow_threshold){
        return 1.0;
    } else {
        return 0.4;
    }
}

vec3 compute_indirect_illumination(vec3 frag_normalized_normal){
    vec3 frag_light_space_coord = light_frag_pos.xyw;
    frag_light_space_coord.xy = frag_light_space_coord.xy/frag_light_space_coord.z * 0.5 + 0.5;
    vec3 indirect = vec3(0.0);

    for(int i = 0; i <= samples_number; i++){
        vec3 random_sample = texelFetch(sample_array, i, 0).rgb;
        vec3 sampling_coords = vec3(frag_light_space_coord.x + random_sample.x * max_radius,
                                    frag_light_space_coord.y + random_sample.y * max_radius,
                                    frag_light_space_coord.z);

        vec3 vpl_pos = texture(position_map, sampling_coords.xy).rgb;
        vec3 vpl_norm = texture(normal_map, sampling_coords.xy).rgb;
        vec3 vpl_flux = texture(flux_map, sampling_coords.xy).rgb;

        //vec3 vpl_to_frag = frag_pos - (vpl_pos - 10.0 * vpl_norm);
        vec3 vpl_to_frag = frag_pos - vpl_pos;
        float distance_to_vpl = length(vpl_to_frag);
        vec3 result = vpl_flux *
                    max(0.0, dot(vpl_norm, vpl_to_frag)) *
                    max(0.0, dot(frag_normalized_normal, -vpl_to_frag)) /
                    pow(distance_to_vpl, 4.0);
        indirect = indirect + result * random_sample.z;
    }
    return clamp(indirect, 0.0, 1.0);
}

void main(){
    //  common data
    vec3 n = normalize(normal);
    vec3 frag_to_light = scene_light.position - frag_pos;
    float distance_from_light = length(frag_to_light);
    vec3 l = normalize(frag_to_light);
    vec3 camera_to_frag = normalize(frag_pos - camera_position);
    vec3 v = - normalize(camera_to_frag);

    //  attenuation computation
    float attenuation_factor = 1.0/(scene_light.constant_attenuation +
                                    scene_light.linear_attenuation * distance_from_light +
                                    scene_light.quadratic_attenuation * distance_from_light * distance_from_light);

    //  spotlight specific
    float angle_between_light_dir_and_light_to_frag = dot(scene_light.direction, -l);
    float epsilon = scene_light.cutoff_angle - scene_light.outer_cutoff_angle;
    float spotlight_intensity = clamp((angle_between_light_dir_and_light_to_frag - scene_light.outer_cutoff_angle)
                                        / epsilon,
                                        0.0, 1.0);


    //  shadow factor
    float shadow_factor = compute_shadow(distance_from_light);

    //  indirect lighting
    vec3 indirect_component = compute_indirect_illumination(n) * indirect_intensity;

    //  diffuse component
    float d = max(dot(n, l), 0.0);
    d = d * attenuation_factor * spotlight_intensity;
    vec3 diffuse_component = d * diffuse_color.rgb * light_intensity;

    //  ambient component
    vec3 ambient_component = ambient_color.xyz * ambient_color.w;

    //  specular component
    vec3 reflection_direction = reflect(-l, n);
    //  beware of NaN when pow(0,0) - delete control and use the following line if you need performance
    //float specular_factor = pow(max(dot(v, reflection_direction), 0.0000000001), shininess);
    float specular_factor = shininess == 0 ? 1.0 : pow(max(dot(v, reflection_direction), 0.0), shininess);
    vec3 specular_component = specular_color.w * specular_color.xyz * specular_factor * spotlight_intensity;

    FragColor = hide_direct_component ?   vec4(indirect_component, 1.0)
                                        :   vec4((diffuse_component + specular_component) * shadow_factor + ambient_component + indirect_component, 1.0);
}
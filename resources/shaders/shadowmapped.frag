#version 430 core

in vec3 frag_pos;
in vec4 light_frag_pos;
in vec3 normal;

out vec4 FragColor;

layout ( location = 6) uniform float opacity;
layout ( location = 7) uniform float shininess;
layout ( location = 8) uniform float refract_i;
layout ( location = 9) uniform vec4 diffuse_color;
layout ( location = 10) uniform vec4 ambient_color;
layout ( location = 11) uniform vec4 specular_color;
layout ( location = 12) uniform vec4 emissive_color;
layout ( location = 13) uniform vec4 transparent_color;

struct Light{
    vec3 position;
    vec3 direction;

    float constant_attenuation;
    float linear_attenuation;
    float quadratic_attenuation;
};

uniform Light scene_light;

layout ( location = 14) uniform samplerCube shadow_map;
layout ( location = 15) uniform samplerCube position_map;
layout ( location = 16) uniform samplerCube normal_map;
layout ( location = 17) uniform samplerCube flux_map;
layout ( location = 18) uniform samplerCube ies_mask;
layout ( location = 19) uniform sampler1D sample_array;

layout ( location = 20) uniform int samples_number;

layout ( location = 21) uniform vec3 camera_position;
layout ( location = 22) uniform float far_plane;
layout ( location = 23) uniform float furthest_photometric_distance;

// Tweakable values
layout ( location = 24) uniform float shadow_threshold;
layout ( location = 25) uniform float max_radius;
layout ( location = 26) uniform float indirect_intensity;
layout ( location = 27) uniform float light_intensity;
layout ( location = 28) uniform bool ies_masking;
layout ( location = 29) uniform bool hide_direct_component;

float compute_shadow(vec3 light_to_frag, float light_distance){
    //  Sample the shadow_map and multiply it for the far plane distance, so you can compare it to the distance
    // of the fragment from the light
    float depth = texture(shadow_map, light_to_frag).r;
    depth *= far_plane;

    if (light_distance < depth + shadow_threshold){
        return 1.0;
    } else {
        return 0.4;
    }
}

vec3 compute_indirect_illumination(vec3 light_to_frag, vec3 frag_normalized_normal){
    vec3 indirect = vec3(0.0);

    for(int i = 0; i <= samples_number; i++){
        vec3 random_sample = texelFetch(sample_array, i, 0).rgb;
        vec3 sampling_direction = normalize(vec3(
                                    light_to_frag.x + random_sample.x * max_radius,
                                    light_to_frag.y + random_sample.y * max_radius,
                                    light_to_frag.z + random_sample.z * max_radius));

        float weight = 1.0 - dot(sampling_direction, light_to_frag);

        vec3 vpl_pos = texture(position_map, sampling_direction).rgb;
        vec3 vpl_norm = texture(normal_map, sampling_direction).rgb;
        vec3 vpl_flux = texture(flux_map, sampling_direction).rgb;

        //vec3 vpl_to_frag = frag_pos - (vpl_pos - 10.0 * vpl_norm);
        vec3 vpl_to_frag = frag_pos - vpl_pos;
        float distance_to_vpl = length(vpl_to_frag);
        vec3 result = vpl_flux *
                    max(0.0, dot(vpl_norm, vpl_to_frag)) *
                    max(0.0, dot(frag_normalized_normal, -vpl_to_frag)) /
                    pow(distance_to_vpl, 4.0);
        indirect = indirect + result * weight;
    }
    return clamp(indirect * 12.566/(float(samples_number)), 0.0, 1.0);
    // or return clamp(indirect, 0.0, 1.0)  * 12.566/(float(samples_number));
}

void main(){
    //  Common data
    vec3 n = normalize(normal);
    vec3 frag_to_light = scene_light.position - frag_pos;
    float distance_from_light = length(frag_to_light);
    vec3 l = normalize(frag_to_light);
    vec3 camera_to_frag = normalize(frag_pos - camera_position);
    vec3 v = - normalize(camera_to_frag);

    //  Compute fragment position in light space and move it in the unit square [0, 1]×[0, 1]
    vec4 fragment_light_space_coordinates = vec4(light_frag_pos.xyz / light_frag_pos.w, light_frag_pos.w);
    fragment_light_space_coordinates.xy = fragment_light_space_coordinates.xy * 0.5 + 0.5;

    //  Attenuation computation
    float attenuation_factor = 1.0/(scene_light.constant_attenuation +
                                    scene_light.linear_attenuation * distance_from_light +
                                    scene_light.quadratic_attenuation * distance_from_light * distance_from_light);

    //  Shadow factor
    float shadow_factor = compute_shadow(-l, distance_from_light);

    //  Indirect lighting
    vec3 indirect_component = compute_indirect_illumination(-l, n) * indirect_intensity * diffuse_color.rgb;

    //  Diffuse component
    float d = max(dot(n, l), 0.0);
    d = d * attenuation_factor;

    vec3 diffuse_component;
    diffuse_component = d * diffuse_color.rgb * light_intensity;
    if(ies_masking == true){
        vec3 mask_value = texture(ies_mask, -l).rgb;
        float scaled_distance = mask_value.r;
        //   Consider using the following line if you want it to scale with the size
        //  of the photometric solid used.
        //      float scaled_distance = mask_value.r;
        //   Use the following version if you want it to be scale indepedent:
        //      float scaled_distance = mask_value.g;
        bool is_active = (mask_value.b == 1.0);
        diffuse_component *= is_active ? scaled_distance : 0.0;
    }

    //  Ambient component
    vec3 ambient_component = ambient_color.xyz * ambient_color.w;

    //  Specular component
    vec3 reflection_direction = reflect(-l, n);
    //  Beware of NaN when pow(0,0) - delete control and use the following line if you need performance
    //      float specular_factor = pow(max(dot(v, reflection_direction), 0.0000000001), shininess);
    float specular_factor = (shininess == 0) ? 1.0 : pow(max(dot(v, reflection_direction), 0.0), shininess);
    vec3 specular_component = specular_color.w * specular_color.xyz * specular_factor;

    FragColor = hide_direct_component ?   vec4(indirect_component, 1.0)
                                        :   vec4((diffuse_component + specular_component) * shadow_factor + ambient_component + indirect_component, 1.0);
}
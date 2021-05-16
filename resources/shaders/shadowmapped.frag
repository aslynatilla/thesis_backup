#version 430 core

in vec3 frag_pos;
in vec3 normal;

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

layout(std140, binding = 2) uniform Material{
    vec4 diffuse_color;
    vec4 ambient_color;
    vec4 specular_color;
    vec4 emissive_color;
    vec4 transparent_color;
    float opacity;
    float shininess;
    float refract_i;
};

layout(std140, binding = 3) uniform CommonData{
    float light_camera_far_plane;
    float distance_to_furthest_ies_vertex;
    bool is_using_ies_masking;
};

layout ( location = 0) uniform samplerCube shadow_map;
layout ( location = 1) uniform samplerCube position_map;
layout ( location = 2) uniform samplerCube normal_map;
layout ( location = 3) uniform samplerCube flux_map;
layout ( location = 4) uniform samplerCube ies_mask;
layout ( location = 5) uniform sampler1D sample_array;

layout ( location = 6) uniform int VPL_samples_per_fragment;
layout ( location = 7) uniform vec3 camera_position;

// Tweakable values
layout ( location = 8) uniform float shadow_threshold;
layout ( location = 9) uniform float displacement_sphere_radius;
layout ( location = 10) uniform float indirect_intensity;
layout ( location = 11) uniform bool hide_direct_component;

float compute_shadow(vec3 light_to_frag, float light_distance){
    //  Sample the shadow_map and multiply it for the far plane distance, so you can compare it to the distance
    // of the fragment from the light
    float depth = texture(shadow_map, light_to_frag).r;
    depth *= light_camera_far_plane;

    if (light_distance < depth + shadow_threshold){
        return 1.0;
    } else {
        return 0.0;
    }
}

vec3 compute_indirect_illumination(vec3 light_to_frag, vec3 frag_normalized_normal){
    vec3 indirect = vec3(0.0);

    for(int i = 0; i <= VPL_samples_per_fragment; i++){
        vec3 random_sample = texelFetch(sample_array, i, 0).rgb;
        vec3 sampling_direction = normalize(vec3(
                                    light_to_frag.x + random_sample.x * displacement_sphere_radius,
                                    light_to_frag.y + random_sample.y * displacement_sphere_radius,
                                    light_to_frag.z + random_sample.z * displacement_sphere_radius));

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
        //return clamp(indirect * 12.566/(float(VPL_samples_per_fragment)), 0.0, 1.0);
        // or return clamp(indirect, 0.0, 1.0)  * 12.566/(float(VPL_samples_per_fragment));
        return indirect * 12.566/(float(VPL_samples_per_fragment));
    }

    vec3 F_Schlick(vec3 f0, float f90, float u){
        return f0 + (f90-f0) * pow(1.0 - u, 5.0);
    }

    void main(){
        //  Common data
        vec3 n = normalize(normal);
        vec3 frag_to_light = scene_light.position.xyz - frag_pos;
        float distance_from_light = length(frag_to_light);
        vec3 l = normalize(frag_to_light);
        vec3 camera_to_frag = normalize(frag_pos - camera_position);
        vec3 v = - normalize(camera_to_frag);
        vec3 h = normalize(v+l);

        //  Attenuation computation
        //  float attenuation_factor = 1.0/(scene_light.constant_attenuation +
        //                                scene_light.linear_attenuation * distance_from_light +
        //                                scene_light.quadratic_attenuation * distance_from_light * distance_from_light);

        float square_distance = distance_from_light * distance_from_light;
        float attenuation_factor = 1.0/(max(square_distance, 0.01 * 0.01));
        attenuation_factor *= 1/12.566;
        //  Shadow factor
        float shadow_factor = compute_shadow(-l, distance_from_light);

        //  Indirect lighting
        vec3 indirect_component = compute_indirect_illumination(-l, n) * indirect_intensity * diffuse_color.rgb;

        //  Diffuse component
        float d = max(dot(n, l), 0.0);
        d = d * attenuation_factor;

        //Disney addition
        float roughness = 0.2;
        float energy_bias = mix(0.0, 0.5, roughness);
        float energy_factor = mix(1.0, 1.0/1.51, roughness);
        float LdotH = dot(l, h);
        float fd90 = energy_bias + 2.0 * LdotH * LdotH * roughness;
        vec3 f0 = vec3(1.0);
        float light_scatter = F_Schlick(f0, fd90, dot(n, l)).r;
        float view_scatter = F_Schlick(f0, fd90, dot(n, v)).r;
        float disney_factor = light_scatter * view_scatter * energy_factor;
        d *= disney_factor / 3.14;

        vec3 diffuse_component;
        diffuse_component = d * diffuse_color.rgb * scene_light.intensity;
        if(is_using_ies_masking == true){
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

        vec4 oc = hide_direct_component ?   vec4(indirect_component, 1.0)
                                            :   vec4((diffuse_component + specular_component) * shadow_factor + ambient_component + indirect_component, 1.0);
        //  IMHO this is the worst of the two techniques:
        //      FragColor = vec4(vec3(1.0) - exp(-oc.rgb * 1.0), 1.0);
        //  Reinhard looks lighter and more effective:
        FragColor = vec4(oc.rgb / (oc.rgb + vec3(1.0)), 1.0);
}
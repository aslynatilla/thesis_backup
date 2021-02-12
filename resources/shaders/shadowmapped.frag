#version 330 core

in vec3 frag_pos;
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

uniform samplerCube shadow_map;
uniform samplerCube position_map;
uniform samplerCube normal_map;
uniform samplerCube flux_map;
uniform sampler1D sample_array;
uniform isamplerCube face_selector;  //isamplerCube

uniform int samples;
uniform int max_sample_radius;

uniform mat4 light_view_projection;

uniform float far_plane;

float compute_shadow(vec3 light_to_fragment, float light_distance)
{
    float depth = texture(shadow_map, light_to_fragment).r;
    depth *= far_plane;
    if (light_distance < depth + 0.5){
        return 1.0;
    } else {
        return 0.5;
    }
}

vec3 compute_indirect(vec3 light_to_fragment, vec3 fragment_pos, vec3 fragment_norm)
{
    ivec2 offset_components = texture(face_selector, light_to_fragment).rg;
    vec2 offset_signs = vec2(sign(offset_components[0]), sign(offset_components[1]));
    int first_component = abs(offset_components[0]);
    int second_component = abs(offset_components[1]);

    vec3 accumulator = vec3(0.0);
    vec3 offsetter = vec3(0.0);
    for(int i = 0; i < samples; ++i){
        vec2 offset = texture(sample_array, i).rg;
        //  note that max offset is (1.0, 1.0)
        offsetter[first_component] = offset_signs[0] * offset[0];
        offsetter[second_component] = offset_signs[1] * offset[1];

        vec3 vpl_pos = texture(position_map, light_to_fragment + offsetter).rgb;
        vec3 vpl_normal = texture(normal_map, light_to_fragment + offsetter).rgb;
        vec3 flux = texture(flux_map, light_to_fragment + offsetter).rgb;

        float irradiance_first_comp = max(0, dot(vpl_normal, fragment_pos - vpl_pos));
        float irradiance_second_comp = max(0, dot(fragment_norm, vpl_pos - fragment_pos));
        float irradiance_denom = pow(length(fragment_pos - vpl_pos), 4);

        accumulator += flux * irradiance_first_comp * irradiance_second_comp * offset.x * offset.x / irradiance_denom;
    }
    return clamp(accumulator, 0.0, 1.0);
}

void main(){
    //  common data
    vec3 n = normalize(normal);
    vec3 frag_to_light = light_position - frag_pos;
    float distance_from_light = length(frag_to_light);
    vec3 l = normalize(frag_to_light);
    float attenuation_factor = 1.0/(1.0 + 0.003 * distance_from_light + 0.00005 * distance_from_light * distance_from_light);
    vec3 camera_to_frag = normalize(frag_pos - camera_position);
    vec3 v = - normalize(camera_to_frag);

    //  shadow factor
    float shadow_factor = compute_shadow(-frag_to_light, distance_from_light);

    //  diffuse component
    float d = max(dot(n, l), 0.0);
    d = d * attenuation_factor;
    vec3 diffuse_component = d * vec3(1.0);

    //  ambient component
    vec3 ambient_component = ambient_color.xyz * ambient_color.w;

    //  specular component
    vec3 reflection_direction = reflect(-l, n);
    //  beware of NaN when pow(0,0) - delete control and use the following line if you need performance
    //float specular_factor = pow(max(dot(v, reflection_direction), 0.0000000001), shininess);
    float specular_factor = shininess == 0 ? 1.0 : pow(max(dot(v, reflection_direction), 0.0), shininess);
    vec3 specular_component = specular_color.w * specular_color.xyz * specular_factor;

    //  indirect component
    vec3 indirect = compute_indirect(-frag_to_light, frag_pos, n);

    FragColor = vec4((diffuse_component + specular_component + indirect) * shadow_factor + ambient_component, 1.0) * diffuse_color;
}

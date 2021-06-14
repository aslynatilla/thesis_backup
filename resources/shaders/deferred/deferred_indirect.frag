#version 430 core

in vec2 uv_coords;

out vec4 indirect_lighting;

layout(std140, binding = 2) uniform Light{
    vec4 position;
    vec4 direction;
    float constant_attenuation;
    float linear_attenuation;
    float quadratic_attenuation;
    float intensity;
    vec4 color;
} scene_light;

layout (location = 0) uniform sampler2D g_positions;
layout (location = 1) uniform sampler2D g_normals;
layout (location = 2) uniform samplerCube rsm_position_map;
layout (location = 3) uniform samplerCube rsm_normal_map;
layout (location = 4) uniform samplerCube rsm_flux_map;
layout (location = 5) uniform sampler1D sampling_offsets;

layout (location = 10) uniform int samples_per_fragment;
layout (location = 11) uniform float displacement_sphere_radius;

void main(){
    vec3 world_position = texture(g_positions, uv_coords).xyz;
    vec3 n = texture(g_normals, uv_coords).xyz;

    vec3 fragment_to_light = scene_light.position.xyz - world_position;
    float distance_from_light = length(fragment_to_light);
    vec3 l = normalize(fragment_to_light);

    vec3 indirect_component = vec3(0.0);

    for(int i=0; i < samples_per_fragment; i++){
        vec3 offset = texelFetch(sampling_offsets, i, 0).rgb;
        vec3 sampling_direction = normalize(
                                    vec3(
                                        -l.x + offset.x * displacement_sphere_radius,
                                        -l.y + offset.y * displacement_sphere_radius,
                                        -l.z + offset.z * displacement_sphere_radius
                                        )
                                    );
        float weight = 1.0 - dot(sampling_direction, -l);

        vec3 vpl_position = texture(rsm_position_map, sampling_direction).rgb;
        vec3 vpl_normal = texture(rsm_normal_map, sampling_direction).rgb;
        vec3 vpl_flux = texture(rsm_flux_map, sampling_direction).rgb;

        vec3 vpl_to_fragment = world_position - vpl_position;
        float d = length(vpl_to_fragment);
        float d2 = d*d;
        vec3 result = vpl_flux *
                        max(0.0, dot(vpl_normal, vpl_to_fragment)) *
                        max(0.0, dot(n, -vpl_to_fragment)) /
                        (d2 * d2);
        indirect_component += result * weight;
    }

    indirect_lighting = vec4(clamp(indirect_component, 0.0, 1.0) * 12.566/(float(samples_per_fragment)), 1.0);
}
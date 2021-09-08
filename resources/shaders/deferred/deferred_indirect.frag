#version 430 core
#define NUMBER_OF_LIGHTS 1

in vec2 uv_coords;

out vec4 indirect_lighting;

layout(std140, binding = 2) uniform Lights{
    vec4 positions[NUMBER_OF_LIGHTS];
    vec4 directions[NUMBER_OF_LIGHTS];
    vec4 attenuations_and_intensities[NUMBER_OF_LIGHTS];    //constant, linear, quadratic attenuation and intensity
    vec4 colors[NUMBER_OF_LIGHTS];
} scene_lights;

layout (location = 0) uniform sampler2D g_positions;
layout (location = 1) uniform sampler2D g_normals;
layout (location = 2) uniform sampler2D g_diffuse_colors;
layout (location = 3) uniform sampler1D sampling_offsets;
layout (location = 4) uniform int samples_per_fragment;

layout (location = 5) uniform samplerCube rsm_position_maps[NUMBER_OF_LIGHTS];

//      The following ones are      5 + NUMBER_OF_LIGHTS * 1
//                                  5 + NUMBER_OF_LIGHTS * 2
//      ...and so on.

layout (location = 6) uniform samplerCube rsm_normal_maps[NUMBER_OF_LIGHTS];
layout (location = 7) uniform samplerCube rsm_flux_maps[NUMBER_OF_LIGHTS];
layout (location = 8) uniform float displacement_sphere_radiuses[NUMBER_OF_LIGHTS];

void main(){
    vec3 world_position = texture(g_positions, uv_coords).xyz;
    vec3 n = texture(g_normals, uv_coords).xyz;

    vec3 ls[NUMBER_OF_LIGHTS];
    for(int i = 0; i < NUMBER_OF_LIGHTS; ++i){
        vec3 fragment_to_light = scene_lights.positions[i].xyz - world_position;
        ls[i] = normalize(fragment_to_light);
    }

    vec3 diffuse_color = texture(g_diffuse_colors, uv_coords).xyz;

    vec3 indirect_component = vec3(0.0);
    float normalizer = 1/(float(samples_per_fragment));

    for(int j = 0; j < NUMBER_OF_LIGHTS; ++j){
        for(int i=0; i < samples_per_fragment; i++){
            vec3 offset = texelFetch(sampling_offsets, i, 0).rgb;
            vec3 sampling_direction = normalize(
                                        vec3(
                                            -ls[j].x + offset.x * displacement_sphere_radiuses[j],
                                            -ls[j].y + offset.y * displacement_sphere_radiuses[j],
                                            -ls[j].z + offset.z * displacement_sphere_radiuses[j]
                                            )
                                        );
            float weight = (1.0 - dot(sampling_direction, -ls[j])) * normalizer;

            vec3 vpl_position = texture(rsm_position_maps[j], sampling_direction).rgb;
            vec3 vpl_normal = texture(rsm_normal_maps[j], sampling_direction).rgb;
            vec3 vpl_flux = texture(rsm_flux_maps[j], sampling_direction).rgb;

            vec3 vpl_to_fragment = world_position - vpl_position;
            float d = length(vpl_to_fragment);
            float d2 = d*d;
            vec3 result = vpl_flux *
                            max(0.0, dot(vpl_normal, vpl_to_fragment)) *
                            max(0.0, dot(n, -vpl_to_fragment)) /
                            (d2 * d2);
            indirect_component += result * weight;
        }
        indirect_lighting += vec4(clamp(indirect_component, 0.0, 1.0) * diffuse_color, 0.0);
    }
    indirect_lighting = vec4(indirect_lighting.rgb, 1.0);
}
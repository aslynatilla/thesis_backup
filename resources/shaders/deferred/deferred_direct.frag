#version 430 core

in vec2 uv_coords;

out vec4 direct_lighting;

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
} scene_light;

layout(std140, binding = 3) uniform CommonData{
    vec4 camera_position;
    float light_camera_far_plane;
    float shadow_threshold;
};

layout (location = 0) uniform sampler2D g_positions;
layout (location = 1) uniform sampler2D g_normals;
layout (location = 2) uniform sampler2D g_diffuse;

layout (location = 3) uniform samplerCube light_shadow_map;
layout (location = 4) uniform samplerCube ies_masking_texture;

float compute_shadow_factor(vec3 light_to_fragment, float distance_from_light){
    float depth = texture(light_shadow_map, light_to_fragment).r;
    depth *= light_camera_far_plane;

    if(distance_from_light < depth + shadow_threshold){
        return 1.0;
    } else {
        return 0.0;
    }
}

void main(){
    vec3 world_position = texture(g_positions, uv_coords).xyz;
    vec3 n = texture(g_normals, uv_coords).xyz;
    vec3 diffuse_color = texture(g_diffuse, uv_coords).xyz;

    vec3 fragment_to_light = scene_light.position.xyz - world_position;
    float distance_from_light = length(fragment_to_light);
    vec3 l = normalize(fragment_to_light);

    float shadow_factor = compute_shadow_factor(-l, distance_from_light);

    float attenuation_factor = 1.0/(scene_light.constant_attenuation +
                                    scene_light.linear_attenuation * distance_from_light +
                                    scene_light.quadratic_attenuation * distance_from_light * distance_from_light);

    float d = max(dot(n, l), 0.0);
    d = d * attenuation_factor;
    vec3 diffuse_component = d * diffuse_color * scene_light.intensity;

    vec3 mask_value = texture(ies_masking_texture, -l).rgb;
    float multiplier = mask_value.r;
    bool is_active = (mask_value.b == 1.0);
    diffuse_component *= is_active ? multiplier : 0.0;

    direct_lighting = vec4(diffuse_component * shadow_factor, 1.0);
}

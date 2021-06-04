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
    vec3 camera_position;
};

layout (location = 0) uniform sampler2D g_positions;
layout (location = 1) uniform sampler2D g_normals;
layout (location = 2) uniform sampler2D g_diffuse;

void main(){
    vec3 world_position = texture(g_positions, uv_coords).xyz;
    vec3 n = texture(g_normals, uv_coords).xyz;
    vec3 diffuse_color = texture(g_diffuse, uv_coords).xyz;

    vec3 fragment_to_light = scene_light.position.xyz - world_position;
    float distance_from_light = len(fragment_to_light);
    vec3 l = normalize(fragment_to_light);

    float shadow_factor = compute_shadow_factor(-l, distance_to_light);

    float attenuation_factor = 1.0/(scene_light.constant_attenuation +
                                    scene_light.linear_attenuation * distance_from_light +
                                    scene_light.quadratic_attenuation * distance_from_light * distance_from_light);

    float d = max(dot(n, l), 0.0);
    d = d * attenuation_factor;
    vec3 diffuse_component = d * diffuse_color * scene_light.intensity;

    direct_lighting = vec4(diffuse_component * shadow_factor, 1.0);
}

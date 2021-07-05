#version 430 core
#define NUMBER_OF_LIGHTS 2

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
} scene_lights[NUMBER_OF_LIGHTS];

layout(std140, binding = 3) uniform CommonData{
    vec4 camera_position;
    float shadow_threshold;
    float light_camera_far_planes[NUMBER_OF_LIGHTS];
    float distances_to_furthest_ies_vertex[NUMBER_OF_LIGHTS];
};

layout (location = 0) uniform sampler2D g_positions;
layout (location = 1) uniform sampler2D g_normals;
layout (location = 2) uniform sampler2D g_diffuse;

layout (location = 3) uniform samplerCube light_shadow_maps[NUMBER_OF_LIGHTS];
//      5 = 3 + NUMBER_OF_LIGHTS
layout (location = 5) uniform samplerCube ies_masking_textures[NUMBER_OF_LIGHTS];

float compute_shadow_factor(vec3 light_to_fragment, float distance_from_light, int index){
    float depth = texture(light_shadow_maps[index], light_to_fragment).r;
    depth *= light_camera_far_planes[index];

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

    float distances_from_lights[NUMBER_OF_LIGHTS];
    vec3 ls[NUMBER_OF_LIGHTS];

    for(int i = 0; i < NUMBER_OF_LIGHTS; ++i){
        vec3 fragment_to_light = scene_lights[i].position.xyz - world_position;
        distances_from_lights[i] = length(fragment_to_light);
        ls[i] = normalize(fragment_to_light);
    }

    for(int i = 0; i < NUMBER_OF_LIGHTS; ++i){
        float shadow_factor = compute_shadow_factor(-ls[i], distances_from_lights[i], i);
        if(shadow_factor >= 0.0){
            float attenuation_factor = 1.0/(scene_lights[i].constant_attenuation +
                                            scene_lights[i].linear_attenuation * distances_from_lights[i] +
                                            scene_lights[i].quadratic_attenuation * distances_from_lights[i] * distances_from_lights[i]);

            float d = max(dot(n, ls[i]), 0.0);
            d = d * attenuation_factor;
            vec3 diffuse_component = d * diffuse_color * scene_lights[i].intensity;

            vec3 mask_value = texture(ies_masking_textures[i], -ls[i]).rgb;
            float multiplier = mask_value.r;
            bool is_active = (mask_value.b == 1.0);
            diffuse_component *= is_active ? multiplier : 0.0;
            direct_lighting += vec4(diffuse_component * shadow_factor, 1.0);
        }
    }
}

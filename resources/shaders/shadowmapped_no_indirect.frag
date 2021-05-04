#version 430 core

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

uniform int VPL_samples_per_fragment;

uniform float far_plane;

// TWEAKABLES
uniform float shadow_threshold;

uniform float displacement_sphere_radius;
uniform float indirect_intensity;
uniform float light_intensity;


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

    FragColor = vec4((diffuse_component + specular_component) * shadow_factor + ambient_component, 1.0);
}
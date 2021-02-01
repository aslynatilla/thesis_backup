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

uniform vec3 light_position = vec3(278.0, 548.0, 279.5);
uniform vec3 camera_position;

void main(){

    //  ambient component
	vec3 ambient = ambient_color.w * ambient_color.xyz;
	
	vec3 n = normalize(normal);
	vec3 light_distance = light_position - frag_pos;
	float distance_from_light = length(light_distance);
	vec3 l = normalize(light_distance);
	float attenuation_factor = 1.0/(1.0 +
	                                0.0014 * distance_from_light +
	                                0.000007 * distance_from_light * distance_from_light);

    //  diffuse component
	float diff = max(dot(n, l), 0.0);
	vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);

	//  specular component
	vec3 view_direction = normalize(frag_pos - camera_position);
	vec3 v = -view_direction;
	vec3 reflection_direction = reflect(-l, n);
    float specular_factor = pow(max(dot(v, reflection_direction), 0.0), shininess);
    vec3 specular = specular_color.w * specular_factor * specular_color.xyz;

	FragColor = vec4((ambient + diffuse + specular) * attenuation_factor, 1.0) * diffuse_color;
}

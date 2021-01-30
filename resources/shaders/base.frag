#version 330 core

in vec3 frag_pos;
in vec3 normal;

out vec4 FragColor;

uniform vec4 color;
uniform vec3 light_direction;

void main(){
	vec3 ambient = 0.7 * vec3(1.0, 1.0, 1.0);
	
	vec3 n = normalize(normal);

	//fixed light direction
	vec3 l = normalize(light_direction);
	float diff = max(dot(n, l), -0.2);
	vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);
	FragColor = vec4(ambient + diffuse, 1.0) * color;
}

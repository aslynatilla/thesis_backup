#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;

out vec3 frag_pos;
out vec4 light_frag_pos;
out vec3 normal;

uniform mat4 model;
uniform mat4 transpose_inverse_model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 light_view;
uniform mat4 light_projection;

void main(){
	frag_pos = vec3(model * vec4(pos, 1.0));
	gl_Position = projection * view * vec4(frag_pos, 1.0);
	light_frag_pos = light_projection * light_view * vec4(frag_pos, 1.0);
	normal = normalize(mat3(transpose_inverse_model) * norm);
}

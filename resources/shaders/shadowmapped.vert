#version 430 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;

out vec3 frag_pos;
out vec4 light_frag_pos;
out vec3 normal;

layout(std140, binding = 0) uniform TransformationMatrices{
    mat4 model;
    mat4 transpose_inverse_model;
    mat4 view;
    mat4 projection;
};

void main(){
	frag_pos = vec3(model * vec4(pos, 1.0));
	gl_Position = projection * view * vec4(frag_pos, 1.0);
	normal = normalize(mat3(transpose_inverse_model) * norm);
}

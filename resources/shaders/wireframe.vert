#version 430 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec3 frag_position;
out vec3 frag_normal;

layout (location = 0) uniform mat4 model;
layout (location = 1) uniform mat4 transpose_inverse_model;
layout (location = 2) uniform mat4 view;
layout (location = 3) uniform mat4 projection;


void main(){
    frag_position = vec3(model * vec4(position, 1.0));
    gl_Position = projection * view * vec4(frag_position, 1.0);
    frag_normal = mat3(transpose_inverse_model) * normal;
}
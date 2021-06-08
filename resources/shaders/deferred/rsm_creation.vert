#version 430 core

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_normal;

layout(std140, binding = 0) uniform GBufferMatrices{
    uniform mat4 projection_view;               // with P left-multiplied by V, as in P * V
    uniform mat4 model;
    uniform mat4 transposed_inversed_model;
};

out VS_DATA{
    vec3 normal;
} vertex_data;

void main(){
    vec4 fragment_position = model * vec4(vertex_position, 1.0);
    vertex_data.normal = normalize(mat3(transposed_inversed_model) * vertex_normal);
    gl_Position = fragment_position;
}
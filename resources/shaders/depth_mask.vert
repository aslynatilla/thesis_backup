#version 430 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;

layout(std140, binding = 0) uniform TransformationMatrices{
    mat4 model;
    mat4 transpose_inverse_model;
    mat4 view;
    mat4 projection;
};

out vec3 frag_pos;

out vertex_data {
    vec3 normal;
} vdata;

void main()
{
    frag_pos = vec3(model * vec4(pos, 1.0));
    vdata.normal = normalize(mat3(transpose_inverse_model) * norm);
    gl_Position = vec4(frag_pos, 1.0);
}
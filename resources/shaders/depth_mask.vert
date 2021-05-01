#version 430 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;

layout (location = 0) uniform mat4 model;
layout (location = 1) uniform mat4 inversed_transposed_model;

out vec3 frag_pos;

out vertex_data {
    vec3 normal;
} vdata;

void main()
{
    frag_pos = vec3(model * vec4(pos, 1.0));
    vdata.normal = normalize(mat3(inversed_transposed_model) * norm);
    gl_Position = vec4(frag_pos, 1.0);
}
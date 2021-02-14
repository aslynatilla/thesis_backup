#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 light_view;
uniform mat4 light_projection;

out vec3 frag_pos;
out vec3 frag_normal;

void main()
{
    frag_pos = vec3(model * vec4(position, 1.0));
    frag_normal = mat3(transpose(inverse(model))) * normal;
    gl_Position = light_projection * light_view * vec4(frag_pos, 1.0);
}
#version 430 core

in vec3 world_position;
in vec3 world_normal;

layout (std140, binding = 1) uniform MaterialProperties{
    vec4 diffuse_color;
    float shininess;
};

layout (location = 2) out vec4 gbuff_diffuse;

void main(){
    gbuff_diffuse = diffuse_color;
}
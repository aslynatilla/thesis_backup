#version 430 core

in vec3 world_position;
in vec3 world_normal;

layout (std140, binding = 1) uniform MaterialProperties{
    vec4 diffuse_color;
    float shininess;
};

layout (location = 0) out vec4 wireframe_overlay;

void main(){
    wireframe_overlay = diffuse_color;
}
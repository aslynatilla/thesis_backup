#version 430 core

in vec3 world_position;
in vec3 world_normal;
in vec2 uv_coords;

layout (std140, binding = 1) uniform MaterialProperties{
    vec4 diffuse_color;
    float shininess;
};

layout (location = 0) uniform sampler2D diffuse_texture;

layout (location = 0) out vec4 gbuff_position;
layout (location = 1) out vec4 gbuff_normal;
layout (location = 2) out vec4 gbuff_diffuse;

void main(){
    gbuff_position = vec4(world_position, 1.0);
    gbuff_normal = vec4(world_normal, 1.0);
    vec4 texture_color = texture(diffuse_texture, uv_coords).rgba;
    gbuff_diffuse = diffuse_color * texture_color;
}
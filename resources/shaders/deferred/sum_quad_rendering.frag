#version 430 core

in vec2 uv_coords;

layout (location = 0) uniform sampler2D first_image;
layout (location = 1) uniform sampler2D second_image;

out vec4 color;

void main(){
    color = vec4(texture(first_image, uv_coords).rgb + texture(second_image, uv_coords).rgb, 1.0);
}
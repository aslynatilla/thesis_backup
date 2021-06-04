#version 430 core

in vec2 uv_coords;

layout (location = 0) uniform sampler2D image_to_quad;

out vec4 color;

void main(){
    color = texture(image_to_quad, uv_coords);
}
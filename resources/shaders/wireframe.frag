#version 430 core

in vec3 frag_position;
in vec3 frag_normal;

layout (location = 4) uniform vec4 wireframe_color;

out vec4 FragColor;

void main(){
    FragColor = wireframe_color;
}
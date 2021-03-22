#version 330 core

in vec3 frag_position;
in vec3 frag_normal;

uniform vec4 wireframe_color;

out vec4 FragColor;

void main(){
    //  FragColor = wireframe_color;
    FragColor = vec4(frag_normal, 1.0);
}
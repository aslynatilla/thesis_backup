#version 330 core

in vec3 frag_pos;
in vec3 frag_normal;

uniform vec4 diffuse_color;
uniform vec3 light_position;
uniform float far_plane;

layout (location = 0) out vec3 fragment_world_coordinates;
layout (location = 1) out vec3 fragment_normal;
layout (location = 2) out vec3 fragment_flux;

void main(){
    float light_distance = length(frag_pos.xyz - light_position);
    gl_FragDepth = light_distance / far_plane;
    float attenuation_factor = 1.0/(1.0 + 0.003 * light_distance + 0.00005 * light_distance * light_distance);

    fragment_world_coordinates = frag_pos;
    fragment_normal = frag_normal;
    fragment_flux = diffuse_color.xyz * attenuation_factor;
}
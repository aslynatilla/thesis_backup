#version 330 core

in vec3 frag_pos;
in vec3 normal;

out float FragColor;

uniform vec3 light_position;

void main()
{
    vec3 light_to_fragment = frag_pos - light_position;
    float distance_from_light = length(light_to_fragment);
    FragColor = distance_from_light;
}
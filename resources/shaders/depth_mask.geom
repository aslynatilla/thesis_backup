#version 430 core

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

in vertex_data {
    vec3 normal;
} input_vertices_data[];

layout (location = 2) uniform mat4 light_transforms[6];

out vec3 frag_pos;
out vec3 frag_normal;

void main()
{
    for(int face=0; face < 6; ++face){
        gl_Layer = face;

        for(int i = 0; i < 3; ++i){
            frag_pos = vec3(gl_in[i].gl_Position);
            gl_Position = light_transforms[face] * gl_in[i].gl_Position;
            frag_normal = input_vertices_data[i].normal;
            EmitVertex();
        }
        EndPrimitive();
    }

}
#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 light_transforms[6];

out vec4 frag_pos;
out vec4 light_space_frag_pos;

void main(){
    for(int face = 0; face < 6; ++face){
        // built-in variable that specifies to which face we render
        gl_Layer = face;

        // for each triangle's vertices
        for(int i = 0; i < 3; ++i){
            frag_pos = gl_in[i].gl_Position;
            light_space_frag_pos = light_transforms[face] * frag_pos;
            gl_Position = light_space_frag_pos;
            EmitVertex();
        }
        EndPrimitive();
    }
}
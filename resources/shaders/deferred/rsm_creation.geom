#version 430

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

in VS_DATA{
    vec3 normal;
} input_vertices_data[];

layout (location = 0) uniform mat4 light_view_projection_transforms[6];

out vec4 fragment_position;
out vec4 light_space_fragment_position;
out vec3 fragment_normal;

void main(){
    for (int face = 0; face < 6; ++face) {
        gl_Layer = face;

        for(int i = 0; i < 3; ++i){
            fragment_position = gl_in[i].gl_Position;
            light_space_fragment_position = light_view_projection_transforms[face] * fragment_position;
            gl_Position = light_space_fragment_position;
            fragment_normal = input_vertices_data[i].normal;
            EmitVertex();
        }
        EndPrimitive();
    }
}
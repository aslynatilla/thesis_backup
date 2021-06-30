#version 430 core

in vec3 vertex_position;
in vec3 vertex_normal;
in vec2 vertex_uvs;

layout(std140, binding = 0) uniform GBufferMatrices{
    uniform mat4 projection_view;               // with P left-multiplied by V, as in P * V
    uniform mat4 model;
    uniform mat4 transposed_inversed_model;
};

//   This will be needed to write light-space positions on a cubemap;
//  and this will need to be moved to a geometry shader
//      uniform mat4 light_view_projection[6];

out vec3 world_position;
out vec3 world_normal;
out vec2 uv_coords;

void main(){
    vec4 w_position = model * vec4(vertex_position, 1.0);

    world_position = w_position.xyz;
    world_normal = normalize(mat3(transposed_inversed_model) * vertex_normal);
    gl_Position = projection_view * w_position;
    uv_coords = vertex_uvs;
}
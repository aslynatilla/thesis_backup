#version 430 core

 layout (location = 0) in vec3 position;
 layout (location = 1) in vec3 normal;

layout(std140, binding = 0) uniform TransformationMatrices{
    mat4 model;
    mat4 transpose_inverse_model;
    mat4 view;
    mat4 projection;
};

 out vec4 frag_pos;
 out VS_OUT{
  vec3 normal;
 } vertex_data;

 void main()
 {
     frag_pos = model * vec4(position, 1.0);
     vertex_data.normal = normalize(mat3(transpose(inverse(model))) * normal);
     gl_Position = frag_pos;
 }
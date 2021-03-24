#version 330 core

 layout (location = 0) in vec3 position;
 layout (location = 1) in vec3 normal;

 uniform mat4 model;
 uniform mat4 light_view;
 uniform mat4 light_projection;

 out vec4 frag_pos;
 out vec3 frag_normal;
 out vec4 light_space_frag_pos;

 void main()
 {
     frag_pos = model * vec4(position, 1.0);
     frag_normal = normalize(mat3(transpose(inverse(model))) * normal);
     light_space_frag_pos = light_projection * light_view * frag_pos;
     gl_Position = light_space_frag_pos;
 }
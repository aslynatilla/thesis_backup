#version 330 core

 layout (location = 0) in vec3 position;
 layout (location = 1) in vec3 normal;

 uniform mat4 model;

 out vec4 frag_pos;
 out vec3 frag_normal;

 void main()
 {
     frag_pos = model * vec4(position, 1.0);
     frag_normal = normalize(mat3(transpose(inverse(model))) * normal);
     gl_Position = frag_pos;
 }
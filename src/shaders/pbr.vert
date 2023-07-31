#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

layout (std140, binding = 0) uniform Matrices 
{
    mat4 projection;
    mat4 view;
};

uniform mat4 model;

out VERT_OUT {
    vec3 FragPos; 
    vec3 Normal;
    vec2 TexCoords;
} vert_out;


void main()
{
    vert_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vert_out.Normal = transpose(inverse(mat3(model))) * aNormal;
    vert_out.TexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
}

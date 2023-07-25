#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

layout (std140, binding = 0) uniform Matrices 
{
    mat4 projection;
    mat4 view;
};

struct DirLight {
    vec3 direction;
    float pad1;
    vec3 ambient;
    float pad2;
    vec3 diffuse;
    float pad3;
    vec3 specular;
    float pad4;
    mat4 lightSpaceMatrix;
};

struct PointLight {    
    vec3 position;
    float constant;
    vec3 ambient;
    float linear;
    vec3 diffuse;
    float quadratic;
    vec3 specular;
    float far;
    mat4 shadowTransforms[6];
};

#define NR_POINT_LIGHTS 3  
layout (std140, binding = 1) uniform Lights 
{
    DirLight dirLight;
    PointLight pointLights[NR_POINT_LIGHTS];
};

uniform mat4 model;
//uniform mat4 lightSpaceMatrix;

out VERT_OUT {
    vec3 FragPos; 
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} vert_out;


void main()
{
    vert_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vert_out.Normal = transpose(inverse(mat3(model))) * aNormal;
    vert_out.TexCoords = aTexCoords;
    vert_out.FragPosLightSpace = dirLight.lightSpaceMatrix * vec4(vert_out.FragPos, 1.0);
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
}

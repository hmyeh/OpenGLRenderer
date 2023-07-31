#version 460 core
layout (location = 0) in vec3 aPos;

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

void main()
{
    gl_Position = dirLight.lightSpaceMatrix * model * vec4(aPos, 1.0);
}

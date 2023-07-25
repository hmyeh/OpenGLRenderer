#version 460 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

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

uniform int pointLightIdx;
//uniform mat4 shadowMatrices[6];

out vec4 FragPos; // FragPos from GS (output per emitvertex)

void main()
{
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face; // built-in variable that specifies to which face we render.
        for(int i = 0; i < 3; ++i) // for each triangle vertex
        {
            FragPos = gl_in[i].gl_Position;
            gl_Position = pointLights[pointLightIdx].shadowTransforms[face] * FragPos;
            EmitVertex();
        }    
        EndPrimitive();
    }
} 

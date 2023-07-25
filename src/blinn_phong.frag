#version 460 core
out vec4 FragColor;

in VERT_OUT {
    vec3 FragPos; 
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} frag_in;


struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
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


uniform Material material;
uniform vec3 viewPos;
// Directional light shadowmap
uniform sampler2D depthMap;
// Point light shadowmaps
uniform samplerCube cubeDepthMap[NR_POINT_LIGHTS];


float DirLightShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to NDC
    projCoords = projCoords * 0.5 + 0.5; 
    float currentDepth = projCoords.z; 
    float bias = 0.005;
    // Simple Percentage-Closer Filtering 
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(depthMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(depthMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;

    if(projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}


// Uses omnidirectional shadowmap (cubemap)
float PointLightShadowCalculation(vec3 fragPos, int pointLightIndex)
{
    vec3 fragToLight = fragPos - pointLights[pointLightIndex].position; 
    float closestDepth = texture(cubeDepthMap[pointLightIndex], fragToLight).r;
    closestDepth *= pointLights[pointLightIndex].far;
    float currentDepth = length(fragToLight); 
    float shadow  = 0.0;
    float bias    = 0.005; 
    float samples = 4.0;
    float offset  = 0.1;
    for(float x = -offset; x < offset; x += offset / (samples * 0.5))
    {
        for(float y = -offset; y < offset; y += offset / (samples * 0.5))
        {
            for(float z = -offset; z < offset; z += offset / (samples * 0.5))
            {
                float closestDepth = texture(cubeDepthMap[pointLightIndex], fragToLight + vec3(x, y, z)).r; 
                closestDepth *= pointLights[pointLightIndex].far;   // undo mapping [0;1]
                if(currentDepth - bias > closestDepth)
                    shadow += 1.0;
            }
        }
    }
    shadow /= (samples * samples * samples);
    return shadow;
}

vec3 testPointShadow(vec3 fragPos, int pointLightIndex) {
    vec3 fragToLight = fragPos - pointLights[pointLightIndex].position; 
    float closestDepth = texture(cubeDepthMap[pointLightIndex], fragToLight).r;
    //closestDepth *= pointLights[pointLightIndex].far;
    return vec3(closestDepth);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, float shadow)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    // combine results
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, frag_in.TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, frag_in.TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, frag_in.TexCoords));

    return ambient + (1.0 - shadow) * (diffuse + specular);
} 


vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    // halfwaydir is from blinn phong upgrade
    vec3 halfwayDir = normalize(lightDir + viewDir);

    //vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    // attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, frag_in.TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, frag_in.TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, frag_in.TexCoords));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return (ambient + (1.0 - shadow) * (diffuse + specular));
} 


void main()
{
    // properties
    vec3 norm = normalize(frag_in.Normal);
    vec3 viewDir = normalize(viewPos - frag_in.FragPos);

    // calculate shadow
    float shadow = DirLightShadowCalculation(frag_in.FragPosLightSpace); 

    // phase 1: Directional lighting
    vec3 result = vec3(0.0);
    //vec3 result = CalcDirLight(dirLight, norm, viewDir, shadow);
    // phase 2: Point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++) {
        float pointShadow = PointLightShadowCalculation(frag_in.FragPos, i);
        result += CalcPointLight(pointLights[i], norm, frag_in.FragPos, viewDir, pointShadow);   
    }
         
    // phase 3: Spot light
    //result += CalcSpotLight(spotLight, norm, frag_in.FragPos, viewDir);    

    FragColor = vec4(result, 1.0);
}

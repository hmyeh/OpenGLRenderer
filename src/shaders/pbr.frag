#version 460 core
out vec4 FragColor;

in VERT_OUT {
    vec3 FragPos; 
    vec3 Normal;
    vec2 TexCoords;
} frag_in;

// material parameters
struct Material {
    sampler2D albedo;
    //vec3 albedo;
    float metallic;
    float roughness;
    float ao;
};

// lights
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

const float PI = 3.14159265359;

uniform Material material;
uniform vec3 viewPos;
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

// Directional light shadowmap
uniform sampler2D depthMap;
// Point light shadowmaps
uniform samplerCube cubeDepthMap[NR_POINT_LIGHTS];


float DirLightShadowCalculation(vec3 fragPos)
{
    vec4 fragPosLightSpace = dirLight.lightSpaceMatrix * vec4(fragPos, 1.0);
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
  
float DistributionGGX(vec3 N, vec3 H, float roughness) 
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
} 


// Cook-terrance BRDF (expect normalized vectors)
vec3 computeBRDF(vec3 F0, vec3 albedo, vec3 L, vec3 V, vec3 N) {
    vec3 H = normalize(V + L);
    float NDF = DistributionGGX(N, H, material.roughness);        
    float G = GeometrySmith(N, V, L, material.roughness);      
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);       
        
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - material.metallic;	  
        
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator; 
    return kD * albedo / PI + specular;
}


void main()
{		
    vec3 N = normalize(frag_in.Normal);
    vec3 V = normalize(viewPos - frag_in.FragPos);
    vec3 R = reflect(-V, N);
    vec3 albedo = texture(material.albedo, frag_in.TexCoords).xyz;

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, material.metallic);
	           
    // reflectance equation
    // Calc directional light reflectance
    vec3 dirL = normalize(-dirLight.direction);
    vec3 Lo = computeBRDF(F0, albedo, dirL, V, N) * dirLight.diffuse * max(dot(N, dirL), 0.0);

    for(int i = 0; i < NR_POINT_LIGHTS; ++i) 
    {
        // calculate per-light radiance
        vec3 L = normalize(pointLights[i].position - frag_in.FragPos);

        float distance    = length(pointLights[i].position - frag_in.FragPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance     = pointLights[i].diffuse * attenuation;        
        
        // cook-torrance brdf
        vec3 brdf = computeBRDF(F0, albedo, L, V, N);

        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);                
        Lo += brdf * radiance * NdotL; 
    }   
  
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, material.roughness);
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - material.metallic;

    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;
    
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  material.roughness * MAX_REFLECTION_LOD).rgb;   
    vec2 envBRDF  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), material.roughness)).rg;
    vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

    vec3 ambient    = (kD * diffuse + specular) * material.ao; 
    vec3 color = ambient + Lo;

    FragColor = vec4(color, 1.0);
}  

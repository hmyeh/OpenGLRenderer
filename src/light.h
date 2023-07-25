#ifndef LIGHT_H
#define LIGHT_H

#include <glad/gl.h> 
#include <glm/glm.hpp>

#include "shader.h"

#include <vector>
#include <type_traits> 

// constants
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;


// std140 layout 4 bytes/vec4
struct PointLight {
    glm::vec3 position;
    float constant;
    glm::vec3 ambient;
    float linear;
    glm::vec3 diffuse;
    float quadratic;
    glm::vec3 specular;
    float far;

    glm::mat4 shadowTransforms[6];

    PointLight() {

    }

    PointLight(const glm::vec3& position, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular,
        float constant, float linear, float quadratic, float far);
};


// Currently padding for std140
struct DirectionalLight {
    glm::vec3 direction;
    float pad1;
    glm::vec3 ambient;
    float pad2;
    glm::vec3 diffuse;
    float pad3;
    glm::vec3 specular;
    float pad4;
    glm::mat4 lightSpaceMatrix;

    DirectionalLight() {}
    DirectionalLight(const glm::vec3& direction, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular);
};


template <class L>
class LightMap {
    unsigned int depthMapFBO;
    unsigned int depthMap;
public:
    LightMap() {
        //static_assert(std::is_same<L, Light>::value, "Works only with Lights!");
        this->generateShadowMap();
    }

    ~LightMap() {
        glDeleteFramebuffers(1, &depthMapFBO);
        glDeleteTextures(1, &depthMap);
    }

    void generateShadowMap();
    // Not the cleanest solution for lightspacematrix computation
    void computeLightSpaceMatrices(L& light, const glm::vec3& bbox = glm::vec3(0.0f));

    unsigned int& getFBO() {
        return depthMapFBO;
    }

    //depthmap/shadowmap
    unsigned int& getDepthMap() {
        return depthMap;
    }
};





class LightingManager {
private:
    DirectionalLight directionalLight;
    std::vector<PointLight> pointLights;

    LightMap<DirectionalLight> directionalLightMap;
    std::vector<std::unique_ptr<LightMap<PointLight> > > pointLightMaps;

    unsigned int uboLights;
public:

    ~LightingManager();

    void setDirectionalLight(const DirectionalLight& directionalLight);
    void addPointLight(const PointLight& pointLight);

    // Use ubo for lighting
    void setupGlBuffers();

    // Bind for the scene draw call
    void bind(Shader& shader, glm::vec3 bbox);
     void configureMatrices(glm::vec3 bbox);
    void bindDirectionalShadowMap();
    void bindPointShadowMap(unsigned int index);
    void releaseShadowMap();

    unsigned int getNumPointLights();

    // For debugging
    unsigned int& getDepthCubemap(int index);
    const PointLight& getPointLight(int index);
};


#endif

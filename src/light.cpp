#include "light.h"

#include <glm/gtc/matrix_transform.hpp>
#include <ostream>

// POINTLIGHT
PointLight::PointLight(const glm::vec3& position, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular,
    float constant, float linear, float quadratic, float far) :
    position(position), constant(constant), ambient(ambient), linear(linear), diffuse(diffuse),
    quadratic(quadratic), specular(specular), far(far) {

}

// DIRECTIONALLIGHT
DirectionalLight::DirectionalLight(const glm::vec3& direction, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular) :
    direction(direction), ambient(ambient), diffuse(diffuse), specular(specular) {

}

template<>
void LightMap<DirectionalLight>::generateShadowMap() {
    // Generate new framebuffer
    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    // z > 1.0 fix
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
};

template<>
void LightMap<DirectionalLight>::computeLightSpaceMatrices(DirectionalLight& light, const glm::vec3& bbox) {
    glm::vec3 sizes = glm::max(bbox, glm::vec3(7.5f));
    //setup matrices for shadowmap
    float near_plane = 1.0f, far_plane = sizes.z;// 7.5f;
    glm::mat4 lightProjection = glm::ortho(-sizes.x, sizes.x, -sizes.y, sizes.y, near_plane, far_plane);

    float eps = 1e-4;
    //glm::vec3 direction = light.direction;
    if (light.direction.x < eps && light.direction.z < eps) {
        light.direction = glm::normalize(light.direction);
        light.direction.x += eps;
    }

    glm::vec3 target = bbox * 0.5f;
    glm::vec3 position = target - light.direction * sizes.z * 0.5f;

    // NOTE!!!:: glm::lookat does not work straight down or up, due to using cross product on (pos-center) x up (parallel vectors) -> 0
    glm::mat4 lightView = glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
    light.lightSpaceMatrix = lightProjection * lightView;
}



template<>
void LightMap<PointLight>::generateShadowMap() {
    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthMap);
    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
            SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

template<>
void LightMap<PointLight>::computeLightSpaceMatrices(PointLight& light, const glm::vec3& bbox) {
    float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
    float near = 0.1f;
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, light.far);

    light.shadowTransforms[0] = shadowProj * glm::lookAt(light.position, light.position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
    light.shadowTransforms[1] = shadowProj * glm::lookAt(light.position, light.position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
    light.shadowTransforms[2] = shadowProj * glm::lookAt(light.position, light.position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
    light.shadowTransforms[3] = shadowProj * glm::lookAt(light.position, light.position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));
    light.shadowTransforms[4] = shadowProj * glm::lookAt(light.position, light.position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));
    light.shadowTransforms[5] = shadowProj * glm::lookAt(light.position, light.position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));
}


//// LightingManager

LightingManager::~LightingManager() {
    glDeleteBuffers(1, &uboLights);
}

void LightingManager::setDirectionalLight(const DirectionalLight& directionalLight) {
    this->directionalLight = directionalLight;
}

void LightingManager::addPointLight(const PointLight& pointLight) {
    this->pointLights.push_back(pointLight);
    this->pointLightMaps.push_back(std::make_unique<LightMap<PointLight> >());
}

// Use ubo for lighting
void LightingManager::setupGlBuffers() {
    glGenBuffers(1, &uboLights);

    glBindBuffer(GL_UNIFORM_BUFFER, uboLights);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(DirectionalLight) + sizeof(PointLight) * pointLights.size(), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 1, uboLights, 0, sizeof(DirectionalLight) + sizeof(PointLight) * pointLights.size());
}


// Bind for the scene draw call
void LightingManager::bind(Shader& shader, glm::vec3 bbox) {
    // Configure light space view matrices
    this->configureMatrices(bbox);

    glBindBuffer(GL_UNIFORM_BUFFER, uboLights);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(DirectionalLight), &directionalLight);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(DirectionalLight), sizeof(PointLight) * pointLights.size(), &pointLights[0]);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Currently using textures from 4 for shadowmaps
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, directionalLightMap.getDepthMap());
    shader.setInt("depthMap", 4);

    for (unsigned int i = 0; i < pointLights.size(); i++) {
        glActiveTexture(GL_TEXTURE5 + i);
        glBindTexture(GL_TEXTURE_CUBE_MAP, pointLightMaps[i]->getDepthMap());

        std::ostringstream os;
        os << "cubeDepthMap[" << i << "]";
        std::string depthMapName = std::move(os).str();
        shader.setInt(depthMapName, 5 + i);
    }
}

void LightingManager::configureMatrices(glm::vec3 bbox) {
    // Point lights
    for (unsigned int i = 0; i < pointLights.size(); i++) {
        pointLightMaps[i]->computeLightSpaceMatrices(pointLights[i]);
    }

    // directional light
    directionalLightMap.computeLightSpaceMatrices(directionalLight, bbox);
}

void LightingManager::bindDirectionalShadowMap() {
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, directionalLightMap.getFBO());
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void LightingManager::bindPointShadowMap(unsigned int index) {
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, pointLightMaps[index]->getFBO());
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void LightingManager::releaseShadowMap() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

unsigned int LightingManager::getNumPointLights() {
    return pointLights.size();
}

unsigned int& LightingManager::getDepthCubemap(int index) {
    return pointLightMaps[index]->getDepthMap();
}

const PointLight& LightingManager::getPointLight(int index) {
    return pointLights[index];
}

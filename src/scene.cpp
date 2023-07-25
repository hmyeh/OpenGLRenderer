#include "scene.h"

Scene::Scene(Camera* camera) : camera(camera) {
    this->cube = std::unique_ptr<Mesh>(new Cube());
    this->plane = std::unique_ptr<Mesh>(new Plane());
    this->stanford_dragon = std::unique_ptr<Mesh>(new TriangleMesh("../resources/xyzrgb_dragon.obj"));

    //hardcoded scene
    items.push_back({ glm::vec3(0.0f, 0.5f, -10.0f), glm::vec3(0.2f), cube.get() });
    items.push_back({ glm::vec3(0.0f, -0.05f, 0.0f), glm::vec3(10.0f), plane.get() });
    items.push_back({ glm::vec3(0.0f), glm::vec3(0.01f), stanford_dragon.get() });

    // hardcoded lights
    lightingManager.setDirectionalLight(DirectionalLight(glm::vec3(0.0f, -4.0f, 0.0f), glm::vec3(0.05f), glm::vec3(0.4f), glm::vec3(0.5f)));
    lightingManager.addPointLight(PointLight(glm::vec3(-2.0f, 4.0f, -1.0f), glm::vec3(0.05f), glm::vec3(0.8f), glm::vec3(1.0f), 1.0f, 0.09f, 0.032f, 25.0f));
    lightingManager.addPointLight(PointLight(glm::vec3(-4.0f, 2.0f, -12.0f), glm::vec3(0.05f), glm::vec3(0.8f), glm::vec3(1.0f), 1.0f, 0.09f, 0.032f, 25.0f));
    lightingManager.addPointLight(PointLight(glm::vec3(0.0f, 0.0f, -2.0f), glm::vec3(0.05f), glm::vec3(0.8f), glm::vec3(1.0f), 1.0f, 0.09f, 0.032f, 25.0f));
    // Setup GL UBO buffers
    lightingManager.setupGlBuffers();

    // currently not a dynamic scene so compute bounding box here
    this->bbox = this->computeBoundingBox();
}


void Scene::draw(Shader& shader) {
    //shader.use();
    for (const SceneItem& item : items) {
        item.mesh->draw(shader, item.position, item.scale);
    }

    //// TRANSPARENT OBJECTS
    //std::map<float, glm::vec3> sorted;
    //for (unsigned int i = 0; i < windows.size(); i++)
    //{
    //    float distance = glm::length(camera->Position - windows[i]);
    //    sorted[distance] = windows[i];
    //}

    //// REVERSE ORDER BACK TO FORWARD TRANSPARENT OBJECT RENDERING
    //for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
    //{
    //    cube.draw(shader, it->second, glm::vec3(0.2f));
    //}
}


void Scene::draw() {
    // draw the lamp object
    lightCubeShader.use();
    for (unsigned int idx = 0; idx < lightingManager.getNumPointLights(); idx++) {
        cube->draw(lightCubeShader, lightingManager.getPointLight(idx).position, glm::vec3(0.2f));
    }


    blinnPhongShader.use();

    lightingManager.bind(blinnPhongShader, this->bbox);

    blinnPhongShader.setVec3("viewPos", camera->Position);

    for (const SceneItem& item : items) {
        item.mesh->draw(blinnPhongShader, item.position, item.scale);
    }

    //// TRANSPARENT OBJECTS
    //std::map<float, glm::vec3> sorted;
    //for (unsigned int i = 0; i < windows.size(); i++)
    //{
    //    float distance = glm::length(camera->Position - windows[i]);
    //    sorted[distance] = windows[i];
    //}

    //transparentShader.use();
    //// REVERSE ORDER BACK TO FORWARD TRANSPARENT OBJECT RENDERING
    //for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
    //{
    //    cube.draw(transparentShader, it->second, glm::vec3(0.2f));
    //}

    //normals
    if (visualize_normals) {
        normalsShader.use();
        stanford_dragon->draw(normalsShader, glm::vec3(0.0f), glm::vec3(0.01));
    }
}

void Scene::bindLightsData(Shader& shader) {
    lightingManager.bind(shader, this->bbox);
}

void Scene::computeShadowMaps() {
    lightingManager.configureMatrices(this->bbox);

    // Compute directional light shadowmap
    depthMapShader.use();
    lightingManager.bindDirectionalShadowMap();
    this->draw(depthMapShader);

    lightingManager.releaseShadowMap();

    // Compute point light shadowmaps
    for (unsigned int i = 0; i < lightingManager.getNumPointLights(); i++) {
        depthCubeMapShader.use();
        depthCubeMapShader.setInt("pointLightIdx", i);

        lightingManager.bindPointShadowMap(i);

        this->draw(depthCubeMapShader);

        lightingManager.releaseShadowMap();
    }
}

glm::vec3 Scene::computeBoundingBox() {
    glm::vec3 bbox(0.0f);
    for (const SceneItem& item : items) {
        glm::vec3 current_bbox = item.mesh->computeBoundingBox(item.scale);
        bbox = glm::max(bbox, current_bbox);
    }
    return bbox;
}


void Scene::setVisualizeNormals(bool visualize_normals) {
    this->visualize_normals = visualize_normals;
}

unsigned int& Scene::getDepthCubemap(int index) {
    return lightingManager.getDepthCubemap(index);
}

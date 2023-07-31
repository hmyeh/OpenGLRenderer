#ifndef SCENE_H
#define SCENE_H

#include <glad/gl.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"
#include "light.h"
#include "mesh.h"
#include "camera.h"

struct SceneItem {
    glm::vec3 position;
    glm::vec3 scale;
    //glm::vec3 rotation;
    Mesh* mesh; // responsibility is on scene class to create the uniqueptr
};

class Scene {
private:
    Camera* camera;
public:
    //lights: dirlight, pointlight
    LightingManager lightingManager;

    // unique meshes/models
    std::unique_ptr<Mesh> cube;
    std::unique_ptr<Mesh> plane;
    std::unique_ptr<Mesh> stanford_dragon;
private:
    // Bounding box (max x, max y, max z)
    glm::vec3 bbox;
    
    // Objects in the scene to draw
    std::vector<SceneItem> items;

    // Create and compile the shaders
    Shader lightCubeShader = Shader("lighting.vert", "lighting.frag");
    Shader normalsShader = Shader("normals.vert", "normals.geom", "normals.frag");
    Shader depthMapShader = Shader("depthmap.vert", "depthmap.frag");
    Shader depthCubeMapShader = Shader("depthcubemap.vert", "depthcubemap.geom", "depthcubemap.frag");
    
    bool visualize_normals = false;


public:
    Scene(Camera* camera);

    void draw(Shader& shader);
    // Special shaders for specific objects different from standard lighting
    void specialShadersDraw();

    void bindLightsData(Shader& shader);
    void computeShadowMaps();
    glm::vec3 computeBoundingBox();

    void setVisualizeNormals(bool visualize_normals);
    unsigned int& getDepthCubemap(int index);
    
};


#endif

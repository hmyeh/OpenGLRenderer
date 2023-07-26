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

    //lights: dirlight, pointlight
    LightingManager lightingManager;

    // unique meshes/models
    std::unique_ptr<Mesh> cube;
    std::unique_ptr<Mesh> plane;
    std::unique_ptr<Mesh> stanford_dragon;
    
    // Bounding box (max x, max y, max z)
    glm::vec3 bbox;
    
    // Objects in the scene to draw
    std::vector<SceneItem> items;

    // Create and compile the shaders
    Shader blinnPhongShader = Shader("../src/blinn_phong.vert", "../src/blinn_phong.frag");
    Shader lightCubeShader = Shader("../src/lighting.vert", "../src/lighting.frag");
    Shader transparentShader = Shader("../src/transparent.vert", "../src/transparent.frag");
    Shader screenShader = Shader("../src/screen.vert", "../src/screen.frag");
    Shader normalsShader = Shader("../src/normals.vert", "../src/normals.geom", "../src/normals.frag");
    Shader instanceShader = Shader("../src/instance.vert", "../src/instance.frag");
    Shader depthMapShader = Shader("../src/depthmap.vert", "../src/depthmap.frag");
    Shader depthCubeMapShader = Shader("../src/depthcubemap.vert", "../src/depthcubemap.geom", "../src/depthcubemap.frag");
    
    bool visualize_normals = false;


public:
    Scene(Camera* camera);

    // TODO: fix the two different draw calls
    void draw(Shader& shader);
    void draw();
    // Special shaders for specific objects different from standard lighting
    void specialShadersDraw();

    void bindLightsData(Shader& shader);
    void computeShadowMaps();
    glm::vec3 computeBoundingBox();

    void setVisualizeNormals(bool visualize_normals);
    unsigned int& getDepthCubemap(int index);
    
};


#endif

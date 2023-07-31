#ifndef MESH_H
#define MESH_H

#include "shader.h"
#include "texture.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

// TODO: transparent objects, texture/material storage, instanced drawing


struct ScreenVertex {
    glm::vec2 pos;
    glm::vec2 tex_coords;
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 tex_coords;
};


// TODO: add albedo texture?
struct Material {
    //unsigned int albedo; // Texture 
    float metallic;
    float roughness;
    float ao;
};


class Mesh {
protected:
    unsigned int VAO, VBO;
    // temp
    Material material = {0.0f, 0.025f, 1.0f};
public:
    ~Mesh();
    virtual void setupGlBuffers() = 0;
    virtual void draw(Shader& shader, const glm::vec3& position, const glm::vec3& scale) = 0;

    // might be replaced after cascaded shadowmapping
    virtual glm::vec3 computeBoundingBox(glm::vec3 scale);
    // TEMP
    void setMetallic(float metallic) {
        this->material.metallic = metallic;
    }
    void setRoughness(float roughness) {
        this->material.roughness = roughness;
    }
    void setAO(float ao) {
        this->material.ao = ao;
    }
};

class ScreenQuad : public Mesh {
    // Normalized Device Coordinates
    const ScreenVertex vertices[6] = {
            {glm::vec2(-1.0f,  1.0f), glm::vec2(0.0f, 1.0f)},
            {glm::vec2(-1.0f, -1.0f), glm::vec2(0.0f, 0.0f)},
            {glm::vec2(1.0f, -1.0f), glm::vec2(1.0f, 0.0f)},

            {glm::vec2(-1.0f,  1.0f), glm::vec2(0.0f, 1.0f)},
            {glm::vec2(1.0f, -1.0f), glm::vec2(1.0f, 0.0f)},
            {glm::vec2(1.0f,  1.0f), glm::vec2(1.0f, 1.0f)},
    };
public:
    ScreenQuad();
    void setupGlBuffers();
    void draw(Shader& shader, const glm::vec3 & /*position*/ = glm::vec3(0.0f), const glm::vec3 & /*scale*/ = glm::vec3(1.0f));

    unsigned int& getVAO();
};


// abstract class cube
class Cube : public Mesh {
private:
    const Vertex vertices[36] = {
        // positions                    //normals                   // texture coords
        // back face
        { glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec2(0.0f, 0.0f) },
        { glm::vec3(1.0f, 1.0f, -1.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec2(1.0f, 1.0f) },
        { glm::vec3(1.0f, -1.0f, -1.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec2(1.0f, 0.0f) },
        { glm::vec3(1.0f, 1.0f, -1.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec2(1.0f, 1.0f) },
        { glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec2(0.0f, 0.0f) },
        { glm::vec3(-1.0f, 1.0f, -1.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec2(0.0f, 1.0f) },
        // front face
        { glm::vec3(-1.0f, -1.0f, 1.0f),  glm::vec3(0.0f,  0.0f, 1.0f),   glm::vec2(0.0f, 0.0f) },
        { glm::vec3(1.0f, -1.0f, 1.0f),  glm::vec3(0.0f,  0.0f, 1.0f),   glm::vec2(1.0f, 0.0f) },
        { glm::vec3(1.0f, 1.0f, 1.0f),  glm::vec3(0.0f,  0.0f, 1.0f),   glm::vec2(1.0f, 1.0f) },
        { glm::vec3(1.0f, 1.0f, 1.0f),  glm::vec3(0.0f,  0.0f, 1.0f),   glm::vec2(1.0f, 1.0f) },
        { glm::vec3(-1.0f, 1.0f, 1.0f),  glm::vec3(0.0f,  0.0f, 1.0f),   glm::vec2(0.0f, 1.0f) },
        { glm::vec3(-1.0f, -1.0f, 1.0f),  glm::vec3(0.0f,  0.0f, 1.0f),   glm::vec2(0.0f, 0.0f) },
        // left face
        { glm::vec3(-1.0f, 1.0f, 1.0f), glm::vec3(-1.0f,  0.0f,  0.0f),  glm::vec2(1.0f, 0.0f) },
        { glm::vec3(-1.0f, 1.0f, -1.0f), glm::vec3(-1.0f,  0.0f,  0.0f),  glm::vec2(1.0f, 1.0f) },
        { glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(-1.0f,  0.0f,  0.0f),  glm::vec2(0.0f, 1.0f) },
        { glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(-1.0f,  0.0f,  0.0f),  glm::vec2(0.0f, 1.0f) },
        { glm::vec3(-1.0f, -1.0f, 1.0f), glm::vec3(-1.0f,  0.0f,  0.0f),  glm::vec2(0.0f, 0.0f) },
        { glm::vec3(-1.0f, 1.0f, 1.0f), glm::vec3(-1.0f,  0.0f,  0.0f),  glm::vec2(1.0f, 0.0f) },
        // right face
        { glm::vec3(1.0f,  1.0f,  1.0f),  glm::vec3(1.0f,  0.0f,  0.0f),  glm::vec2(1.0f, 0.0f) },
        { glm::vec3(1.0f, -1.0f, -1.0f),  glm::vec3(1.0f,  0.0f,  0.0f),  glm::vec2(0.0f, 1.0f) },
        { glm::vec3(1.0f,  1.0f, -1.0f),  glm::vec3(1.0f,  0.0f,  0.0f),  glm::vec2(1.0f, 1.0f) },
        { glm::vec3(1.0f, -1.0f, -1.0f),  glm::vec3(1.0f,  0.0f,  0.0f),  glm::vec2(0.0f, 1.0f) },
        { glm::vec3(1.0f,  1.0f,  1.0f),  glm::vec3(1.0f,  0.0f,  0.0f),  glm::vec2(1.0f, 0.0f) },
        { glm::vec3(1.0f, -1.0f,  1.0f),  glm::vec3(1.0f,  0.0f,  0.0f),  glm::vec2(0.0f, 0.0f) },
        // bottom face
        { glm::vec3(-1.0f, -1.0f, -1.0f),  glm::vec3(0.0f, -1.0f,  0.0f),  glm::vec2(0.0f, 1.0f) },
        { glm::vec3(1.0f, -1.0f, -1.0f),  glm::vec3(0.0f, -1.0f,  0.0f),  glm::vec2(1.0f, 1.0f) },
        { glm::vec3(1.0f, -1.0f,  1.0f),  glm::vec3(0.0f, -1.0f,  0.0f),  glm::vec2(1.0f, 0.0f) },
        { glm::vec3(1.0f, -1.0f,  1.0f),  glm::vec3(0.0f, -1.0f,  0.0f),  glm::vec2(1.0f, 0.0f) },
        { glm::vec3(-1.0f, -1.0f,  1.0f),  glm::vec3(0.0f, -1.0f,  0.0f),  glm::vec2(0.0f, 0.0f) },
        { glm::vec3(-1.0f, -1.0f, -1.0f),  glm::vec3(0.0f, -1.0f,  0.0f),  glm::vec2(0.0f, 1.0f) },
        // top face
        { glm::vec3(-1.0f,  1.0f, -1.0f),  glm::vec3(0.0f,  1.0f,  0.0f),  glm::vec2(0.0f, 1.0f) },
        { glm::vec3(1.0f,  1.0f , 1.0f),  glm::vec3(0.0f,  1.0f,  0.0f),  glm::vec2(1.0f, 0.0f) },
        { glm::vec3(1.0f,  1.0f, -1.0f),  glm::vec3(0.0f,  1.0f,  0.0f),  glm::vec2(1.0f, 1.0f) },
        { glm::vec3(1.0f,  1.0f,  1.0f),  glm::vec3(0.0f,  1.0f,  0.0f),  glm::vec2(1.0f, 0.0f) },
        { glm::vec3(-1.0f,  1.0f, -1.0f),  glm::vec3(0.0f,  1.0f,  0.0f),  glm::vec2(0.0f, 1.0f) },
        { glm::vec3(-1.0f,  1.0f,  1.0f),  glm::vec3(0.0f,  1.0f,  0.0f),  glm::vec2(0.0f, 0.0f) }
    };

public:
    Cube();
    void setupGlBuffers();
    virtual void draw(Shader& shader, const glm::vec3& position, const glm::vec3& scale) = 0; 
    // TODO: might be replaced/removed after cascade shadowmapping
    glm::vec3 computeBoundingBox(glm::vec3 scale);
};


class Skybox : public Cube {
    const glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    const glm::mat4 views[6] =
    {
       glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
       glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
       glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

public:
    Skybox();

    // Ignoring position and scale because skybox
    void draw(Shader& shader, const glm::vec3 & /*position*/ = glm::vec3(0.0f), const glm::vec3 & /*scale*/ = glm::vec3(1.0f));

    const glm::mat4& getProjectionMatrix();
    const glm::mat4& getViewMatrix(int index);

    unsigned int& getVAO();
};


class Plane : public Cube {
private:
    Texture tex = Texture("../resources/floor.png");
    // plane scale (floor)
    glm::vec3 unit_scale = glm::vec3(1.0f, 0.01f, 1.0f);
public:
    Plane();
    void draw(Shader& shader, const glm::vec3& position, const glm::vec3& scale);
};

// Default cube with material
class DefaultCube : public Cube {
private:
    Texture tex = Texture("../resources/white.png");
public:
    DefaultCube();
    void draw(Shader& shader, const glm::vec3& position, const glm::vec3& scale);
};

// Uses https://github.com/tinyobjloader/tinyobjloader
class TriangleMesh : public Mesh {
private:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<unsigned int> material_ids;

    // TODO: read dragon model with textures applied
    Texture albedo = Texture("../resources/white.png");

    unsigned int EBO;

    // ASSUMING TRIANGLE MESH
    void computeVertexNormals();
public:
    TriangleMesh(std::string file_location);
    ~TriangleMesh();

    void loadModel(std::string file_location);

    void setupGlBuffers();
    glm::vec3 computeBoundingBox(glm::vec3 scale);

    void draw(Shader& shader, const glm::vec3& position, const glm::vec3& scale);

};


#endif

#ifndef MESH_H
#define MESH_H

#include "shader.h"
#include "texture.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>


struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 tex_coords;
};

class Mesh {
protected:
    unsigned int VAO, VBO, EBO;
    
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<std::unique_ptr<Texture> > textures;
    
    // true if semi or fully transparent for blending and transparent shader (also back to front order drawing required)
    bool transparent = false;

public:
    ~Mesh();
    void setupGlBuffers();

    //virtual void draw(Shader& shader) = 0;
    virtual void draw(Shader& shader, const glm::vec3& position, const glm::vec3& scale) = 0;

    // TODO: with scale+rotation to get  xmax  ymax zmax?
    glm::vec3 computeBoundingBox(glm::vec3 scale);

    bool isTransparent();
};

class Cube : public Mesh {
private:
    // FOR USAGE WITH INSTANCED RENDERING
    unsigned int instanceVBO;

public:
    Cube();
    void draw(Shader& shader, const glm::vec3& position, const glm::vec3& scale);
    void instancedDraw(Shader& shader, const glm::vec3& position, const glm::vec3& scale);

};

class Plane : public Mesh {
public:
    Plane();
    void draw(Shader& shader, const glm::vec3& position, const glm::vec3& scale);
    
};

// Uses https://github.com/tinyobjloader/tinyobjloader
class TriangleMesh : public Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<unsigned int> material_ids;

public:
    //  render data
    unsigned int VAO, VBO, EBO;

    TriangleMesh(std::string file_location) {
        this->loadModel(file_location);
    }

    void loadModel(std::string file_location);

    // ASSUMING TRIANGLE MESH
    void computeVertexNormals();

    void draw(Shader& shader, const glm::vec3& position, const glm::vec3& scale);
};


#endif

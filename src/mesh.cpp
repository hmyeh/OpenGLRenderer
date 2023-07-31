#include "mesh.h"

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
// Optional. define TINYOBJLOADER_USE_MAPBOX_EARCUT gives robust trinagulation. Requires C++11
//#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include <tiny_obj_loader.h>

// MESH

Mesh::~Mesh() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

glm::vec3 Mesh::computeBoundingBox(glm::vec3 scale) {
    return glm::vec3(0.0f);
}


// Screen Quad
ScreenQuad::ScreenQuad() {
    this->setupGlBuffers();
}

void ScreenQuad::setupGlBuffers() {
    // create buffers/arrays
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);

    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ScreenVertex), (void*)0);
    // vertex texture coords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ScreenVertex), (void*)offsetof(ScreenVertex, tex_coords));
    glBindVertexArray(0);
}

void ScreenQuad::draw(Shader& shader, const glm::vec3 & position, const glm::vec3 & scale) {
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

unsigned int& ScreenQuad::getVAO() {
    return VAO;
}

// Abstract Cube

Cube::Cube() {
    this->setupGlBuffers();
}

void Cube::setupGlBuffers() {
    // create buffers/arrays
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);

    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords));
    glBindVertexArray(0);
}

glm::vec3 Cube::computeBoundingBox(glm::vec3 scale) {
    glm::vec3 bbox(0.0f);
    for (const Vertex& vertex : vertices) {
        glm::vec3 scaled_pos = scale * vertex.pos;
        bbox = glm::max(bbox, glm::abs(scaled_pos));
    }
    return bbox;
}


// Skybox
Skybox::Skybox() : Cube() { }

void Skybox::draw(Shader& shader, const glm::vec3 & position, const glm::vec3 & scale) {
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

const glm::mat4& Skybox::getProjectionMatrix() {
    return this->projection;
}

const glm::mat4& Skybox::getViewMatrix(int index) {
    return this->views[index];
}

unsigned int& Skybox::getVAO() {
    return VAO;
}

// HARDCODED PLANE

Plane::Plane() : Cube() {}

void Plane::draw(Shader& shader, const glm::vec3& position, const glm::vec3& scale) {
    // Set material
    tex.bind(GL_TEXTURE0);
    shader.setInt("material.albedo", 0);
    shader.setFloat("material.metallic", material.metallic);
    shader.setFloat("material.roughness", material.roughness);
    shader.setFloat("material.ao", material.ao);

    // Set Model matrix and draw
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    model = glm::scale(model, glm::vec3(scale) * unit_scale);
    shader.setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}


// Default Cube
DefaultCube::DefaultCube() : Cube() {}

void DefaultCube::draw(Shader& shader, const glm::vec3& position, const glm::vec3& scale) {
    // Set material
    tex.bind(GL_TEXTURE0);
    shader.setInt("material.albedo", 0);
    shader.setFloat("material.metallic", material.metallic);
    shader.setFloat("material.roughness", material.roughness);
    shader.setFloat("material.ao", material.ao);

    // Set Model matrix
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    model = glm::scale(model, glm::vec3(scale));
    shader.setMat4("model", model);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}


// TRIANGLEMESH
TriangleMesh::TriangleMesh(std::string file_location) {
    this->loadModel(file_location);
    this->setupGlBuffers();
}

TriangleMesh::~TriangleMesh() {
    glDeleteBuffers(1, &EBO);
}

void TriangleMesh::loadModel(std::string file_location) {
    tinyobj::ObjReaderConfig reader_config;
    //reader_config.mtl_search_path = dirPath;// "./"; // Path to material files

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(file_location, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        throw std::exception("Failed to read Obj file");
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    int num_verts = attrib.vertices.size() / 3;

    vertices.resize(num_verts);
    // convert to own vertex class from tinyobjloader version
    for (unsigned int i = 0; i < num_verts; i++) {
        vertices[i].pos = glm::vec3(attrib.vertices[3 * i + 0], attrib.vertices[3 * i + 1], attrib.vertices[3 * i + 2]);
        //glm::vec3 normal(attrib.normals[3 * i + 0], attrib.normals[3 * i + 1], attrib.normals[3 * i + 2]);
        if (!attrib.texcoords.empty()) {
            vertices[i].tex_coords = glm::vec2(attrib.texcoords[2 * i + 0], attrib.texcoords[2 * i + 1]);
        }
    }

    // for now merge all shapes into a single mesh
    unsigned int num_faces = 0;
    unsigned int num_indices = 0;
    for (size_t s = 0; s < shapes.size(); s++) {
        // Double-check that triangular mesh
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
            if (fv != 3) {
                std::cerr << "Mesh is not triangular" << std::endl;
            }
        }

        num_faces += shapes[s].mesh.num_face_vertices.size();
        num_indices += shapes[s].mesh.indices.size();
    }

    // for now merge all shapes into a single mesh
    if (!materials.empty()) {
        material_ids.resize(num_faces);
    }

    indices.resize(num_indices);
    unsigned int offset_idx = 0;
    for (size_t s = 0; s < shapes.size(); s++) {
        // Example has separate indices for normal and texcoords, not used here? dont know if problematic
        unsigned int num_mesh_indices = shapes[s].mesh.indices.size();
        for (size_t i = 0; i < num_mesh_indices; i++) {
            indices[offset_idx + i] = shapes[s].mesh.indices[i].vertex_index;
        }

        if (!materials.empty()) {
            // TODO: FIX OFFSET SIMILAR AS ABOVE
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
                material_ids[offset_idx * f] = shapes[s].mesh.material_ids[f];
            }
        }

        offset_idx += num_mesh_indices;
    }

    // In case undefined in obj file
    computeVertexNormals();
}

void TriangleMesh::setupGlBuffers() {
    // create buffers/arrays
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    // Indices in EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords));
    glBindVertexArray(0);
}


void TriangleMesh::computeVertexNormals() {
    unsigned int num_tris = indices.size() / 3;
    for (unsigned int i = 0; i < num_tris; i++) {
        glm::vec3 v1 = vertices[indices[3 * i + 1]].pos - vertices[indices[3 * i + 0]].pos;
        glm::vec3 v2 = vertices[indices[3 * i + 2]].pos - vertices[indices[3 * i + 0]].pos;
        glm::vec3 normal = glm::cross(v1, v2);

        vertices[indices[3 * i + 0]].normal += normal;
        vertices[indices[3 * i + 1]].normal += normal;
        vertices[indices[3 * i + 2]].normal += normal;
    }

    for (unsigned int i = 0; i < vertices.size(); i++) {
        vertices[i].normal = glm::normalize(vertices[i].normal);
    }
}

void TriangleMesh::draw(Shader& shader, const glm::vec3& position, const glm::vec3& scale) {
    // Bind textures
    albedo.bind(GL_TEXTURE0);
    shader.setInt("material.albedo", 0);
    shader.setFloat("material.metallic", material.metallic);
    shader.setFloat("material.roughness", material.roughness);
    shader.setFloat("material.ao", material.ao);

    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    model = glm::scale(model, glm::vec3(scale));
    shader.setMat4("model", model);
    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

glm::vec3 TriangleMesh::computeBoundingBox(glm::vec3 scale) {
    glm::vec3 bbox(0.0f);
    for (const Vertex& vertex : vertices) {
        glm::vec3 scaled_pos = scale * vertex.pos;
        bbox = glm::max(bbox, glm::abs(scaled_pos));
    }
    return bbox;
}

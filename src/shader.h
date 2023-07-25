#ifndef SHADER_H
#define SHADER_H

#include <glad/gl.h> // include glad to get all the required OpenGL headers
#include <glm/glm.hpp>


#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


class Shader
{

    enum class ShaderType {
        VERTEX,
        FRAGMENT,
        GEOMETRY
    };

    unsigned int createShader(const char* file_path, ShaderType shader_type);

public:
    // the program ID
    unsigned int ID;

    // constructor reads and builds the shader
    Shader(const char* vertexPath, const char* fragmentPath);
    Shader(const char* vertexPath, const char* geometryPath, const char* fragmentPath);
    // use/activate the shader
    void use();
    // utility uniform functions
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec3(const std::string& name, float x, float y, float z) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setMat3(const std::string& name, const glm::mat3& value) const;
    void setMat4(const std::string& name, const glm::mat4& value) const;
private:
    void checkCompileErrors(unsigned int shader, std::string type);
};

#endif

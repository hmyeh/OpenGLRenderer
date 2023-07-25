#include "shader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    //// 1. retrieve the vertex/fragment source code from filePath
    //std::string vertexCode;
    //std::string fragmentCode;
    //std::ifstream vShaderFile;
    //std::ifstream fShaderFile;
    //// ensure ifstream objects can throw exceptions:
    //vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    //fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    //try
    //{
    //    // open files
    //    vShaderFile.open(vertexPath);
    //    fShaderFile.open(fragmentPath);
    //    std::stringstream vShaderStream, fShaderStream;
    //    // read file's buffer contents into streams
    //    vShaderStream << vShaderFile.rdbuf();
    //    fShaderStream << fShaderFile.rdbuf();
    //    // close file handlers
    //    vShaderFile.close();
    //    fShaderFile.close();
    //    // convert stream into string
    //    vertexCode = vShaderStream.str();
    //    fragmentCode = fShaderStream.str();
    //}
    //catch (std::ifstream::failure& e)
    //{
    //    std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
    //}
    //const char* vShaderCode = vertexCode.c_str();
    //const char* fShaderCode = fragmentCode.c_str();
    //// 2. compile shaders
    //unsigned int vertex, fragment;
    //// vertex shader
    //vertex = glCreateShader(GL_VERTEX_SHADER);
    //glShaderSource(vertex, 1, &vShaderCode, NULL);
    //glCompileShader(vertex);
    //checkCompileErrors(vertex, "VERTEX");
    //// fragment Shader
    //fragment = glCreateShader(GL_FRAGMENT_SHADER);
    //glShaderSource(fragment, 1, &fShaderCode, NULL);
    //glCompileShader(fragment);
    //checkCompileErrors(fragment, "FRAGMENT");
    unsigned int vertex = this->createShader(vertexPath, ShaderType::VERTEX);
    unsigned int fragment = this->createShader(fragmentPath, ShaderType::FRAGMENT);
    // shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");
    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::Shader(const char* vertexPath, const char* geometryPath, const char* fragmentPath) {
    unsigned int vertex = this->createShader(vertexPath, ShaderType::VERTEX);
    unsigned int geometry = this->createShader(geometryPath, ShaderType::GEOMETRY);
    unsigned int fragment = this->createShader(fragmentPath, ShaderType::FRAGMENT);
    // shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, geometry);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");
    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(geometry);
    glDeleteShader(fragment);
}



unsigned int Shader::createShader(const char* file_path, ShaderType shader_type) {
    std::string code;
    std::ifstream shader_file;
    // ensure ifstream objects can throw exceptions:
    shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // open files
        shader_file.open(file_path);
        std::stringstream shader_stream;
        // read file's buffer contents into streams
        shader_stream << shader_file.rdbuf();
        // close file handlers
        shader_file.close();
        // convert stream into string
        code = shader_stream.str();
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
    }
    const char* shader_code = code.c_str();

    unsigned int shader;
    switch (shader_type) {
    case ShaderType::VERTEX:
        shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(shader, 1, &shader_code, NULL);
        glCompileShader(shader);
        checkCompileErrors(shader, "VERTEX");
        break;
    case ShaderType::FRAGMENT:
        shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(shader, 1, &shader_code, NULL);
        glCompileShader(shader);
        checkCompileErrors(shader, "FRAGMENT");
        break;
    case ShaderType::GEOMETRY:
        shader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(shader, 1, &shader_code, NULL);
        glCompileShader(shader);
        checkCompileErrors(shader, "GEOMETRY");
        break;
    default:
        std::cerr << "Shader type does not exist" << std::endl;
        throw std::exception("Shader type does not exist");
    }

    return shader;
}

void Shader::use()
{
    glUseProgram(ID);
}

// utility uniform functions
    // ------------------------------------------------------------------------
void Shader::setBool(const std::string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
// ------------------------------------------------------------------------
void Shader::setInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
// ------------------------------------------------------------------------
void Shader::setFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}
// ------------------------------------------------------------------------
void Shader::setVec3(const std::string& name, float x, float y, float z) const
{
    glm::vec3 vec(x, y, z);
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(vec));
}
// ------------------------------------------------------------------------
void Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
}
// ------------------------------------------------------------------------
void Shader::setMat3(const std::string& name, const glm::mat3& value) const
{
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}
// ------------------------------------------------------------------------
void Shader::setMat4(const std::string& name, const glm::mat4& value) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}


// utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
void Shader::checkCompileErrors(unsigned int shader, std::string type)
{
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}

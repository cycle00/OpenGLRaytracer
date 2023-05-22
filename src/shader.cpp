#include "shader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "renderer.h"

shader::shader() : m_rendererID(0) {

}

shader::shader(const std::string& filepath) : m_filepath(filepath), m_rendererID(0) {
    shaderProgramSource source = parseShader(filepath);
    m_rendererID = createShader(source.vertexSource, source.fragmentSource);
}

shader::~shader() {
    call(glDeleteProgram(m_rendererID));
}

// Read a shader file and split it up into a vertex and fragment shader
shaderProgramSource shader::parseShader(const std::string& filepath) {
    std::ifstream stream(filepath);

    enum class shaderType {
        NONE = -1,
        VERTEX = 0,
        FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];
    shaderType type = shaderType::NONE;
    while (getline(stream, line)) {
        if (line.find("#shader") != std::string::npos) {
            if (line.find("vertex") != std::string::npos) {
                type = shaderType::VERTEX;
            }
            else if (line.find("fragment") != std::string::npos) {
                type = shaderType::FRAGMENT;
            }
        }
        else if (line.find("//") == std::string::npos) {
            ss[(int)type] << line << '\n';
        }
        else if (int comment = line.find("//")) {
            ss[(int)type] << line.substr(0, comment) << '\n';
        }
    }

    return { ss[0].str(), ss[1].str() };
}

unsigned int shader::compileShader(unsigned int type, const std::string& source) {
    // Compile shader
    call(unsigned int id = glCreateShader(type));
    const char* src = source.c_str();
    call(glShaderSource(id, 1, &src, nullptr));
    call(glCompileShader(id));

    int result;
    call(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    // Handle errors
    if (!result) {
        int length;
        call(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
        char* message = (char*)alloca(length * sizeof(char)); // virgin malloc vs chad alloca
        call(glGetShaderInfoLog(id, length, &length, message));
        std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
        std::cout << message << std::endl;
        call(glDeleteShader(id));
        return 0;
    }

    return id;
}

unsigned int shader::createShader(const std::string& vertexShader, const std::string& fragShader) {
    // Initialize an empty shader and compile vertex and fragment shaders
    call(unsigned int program = glCreateProgram());
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragShader);

    // Use the shaders
    call(glAttachShader(program, vs));
    call(glAttachShader(program, fs));
    call(glLinkProgram(program));
    call(glValidateProgram(program));

    call(glDeleteShader(vs));
    call(glDeleteShader(fs));

    return program;
}

void shader::bind() const {
    call(glUseProgram(m_rendererID));
}

void shader::unbind() const {
    call(glUseProgram(0));
}

void shader::setUniform1i(const std::string& name, int value) {
    call(glUniform1i(getUniformLocation(name), value));
}

void shader::setUniform1f(const std::string& name, float value) {
    call(glUniform1f(getUniformLocation(name), value));
}

void shader::setUniform3f(const std::string& name, float v0, float v1, float v2) {

    call(glUniform3f(getUniformLocation(name), v0, v1, v2));
}

void shader::setUniform4f(const std::string& name, float v0, float v1, float v2, float v3) {
    
    call(glUniform4f(getUniformLocation(name), v0, v1, v2, v3));
}

void shader::setUniformMat4f(const std::string& name, glm::mat4 value) {
    call(glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &value[0][0]));
}

void shader::setUniformObject(scene::object object, unsigned int index) {
    call(glUniform1ui(getUniformLocation(std::string("u_objects[").append(std::to_string(index)).append("].type")), object.type));
    call(glUniform3f(getUniformLocation(std::string("u_objects[").append(std::to_string(index)).append("].position")), object.position[0], object.position[1], object.position[2]));
    call(glUniform3f(getUniformLocation(std::string("u_objects[").append(std::to_string(index)).append("].scale")), object.scale[0], object.scale[1], object.scale[2]));
    // Material
    call(glUniform3f(getUniformLocation(std::string("u_objects[").append(std::to_string(index)).append("].material.albedo")), object.mat.albedo[0], object.mat.albedo[1], object.mat.albedo[2]));
}

void shader::setUniformLight(scene::pointLight light, unsigned int index) {
    call(glUniform3f(getUniformLocation(std::string("u_lights[").append(std::to_string(index)).append("].position")), light.position[0], light.position[1], light.position[2]));
    call(glUniform1f(getUniformLocation(std::string("u_lights[").append(std::to_string(index)).append("].radius")), light.radius));
    call(glUniform3f(getUniformLocation(std::string("u_lights[").append(std::to_string(index)).append("].color")), light.color[0], light.color[1], light.color[2]));
    call(glUniform1f(getUniformLocation(std::string("u_lights[").append(std::to_string(index)).append("].power")), light.power));
    call(glUniform1f(getUniformLocation(std::string("u_lights[").append(std::to_string(index)).append("].reach")), light.reach));
}

int shader::getUniformLocation(const std::string& name) {
    if (m_uniformLocationCache.find(name) != m_uniformLocationCache.end()) {
        return m_uniformLocationCache[name];
    }

    call(int location = glGetUniformLocation(m_rendererID, name.c_str()));
    if (location == -1) {
        std::cout << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;
    }

    m_uniformLocationCache[name] = location;
    
    return location;
}
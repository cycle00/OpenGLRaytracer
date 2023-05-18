#include "shader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "renderer.h"

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
        else {
            ss[(int)type] << line << '\n';
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

void shader::setUniform4f(const std::string& name, float v0, float v1, float v2, float v3) {
    
    call(glUniform4f(getUniformLocation(name), v0, v1, v2, v3));
}

void shader::setUniformMat4f(const std::string& name, glm::mat4 value) {
    call(glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &value[0][0]));
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
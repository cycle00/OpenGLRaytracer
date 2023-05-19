#pragma once

#include <glm/glm.hpp>

#include <string>
#include <unordered_map>

#include "scene.h"

struct shaderProgramSource {
	std::string vertexSource;
	std::string fragmentSource;
};

class shader {
private:
	unsigned int m_rendererID;
	std::string m_filepath;
	std::unordered_map<std::string, int> m_uniformLocationCache;

	shaderProgramSource parseShader(const std::string& filepath);
	unsigned int compileShader(unsigned int type, const std::string& source);
	unsigned int createShader(const std::string& vertexShader, const std::string& fragShader);
	int getUniformLocation(const std::string& name);
public:
	shader(const std::string& filepath);
	~shader();

	void bind() const;
	void unbind() const;

	void setUniform1i(const std::string& name, int value);
	void setUniform1f(const std::string& name, float value);
	void setUniform3f(const std::string& name, float v0, float v1, float v2);
	void setUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
	void setUniformMat4f(const std::string& name, glm::mat4 value);

	void setUniformObject(scene::object object, unsigned int index);
};
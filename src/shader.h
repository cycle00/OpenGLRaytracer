#pragma once

#include <string>
#include <unordered_map>

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

	void setUniform4f(const std::string& name, float f0, float f1, float f2, float f3);
};
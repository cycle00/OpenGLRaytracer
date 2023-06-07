#pragma once

#include "../renderer.h"

class texture {
private:
	unsigned int m_rendererID;
	std::string m_filePath;
	float* m_localBuffer;
	int m_width, m_height, m_bpp;
public:
	texture(const std::string& path);
	~texture();

	void bind(unsigned int slot = 0) const;
	void unbind() const;

	inline int getWidth() const { return m_width; }
	inline int getHeighth() const { return m_height; }
};
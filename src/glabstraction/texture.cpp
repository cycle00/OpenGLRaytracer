#include "texture.h"

#include "../vendor/stb/stb_image.h"

texture::texture(const std::string& path) : m_rendererID(0), m_filePath(path), m_localBuffer(nullptr), m_width(0), m_height(0), m_bpp(0) {
	stbi_set_flip_vertically_on_load(1);
	m_localBuffer = stbi_load(path.c_str(), &m_width, &m_height, &m_bpp, 4);
	
	call(glGenTextures(1, &m_rendererID));
	call(glBindTexture(GL_TEXTURE_2D, m_rendererID));

	call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_localBuffer));
	call(glBindTexture(GL_TEXTURE_2D, 0));

	if (m_localBuffer)
		stbi_image_free(m_localBuffer);
}

texture::~texture() {
	call(glDeleteTextures(1, &m_rendererID));
}

void texture::bind(unsigned int slot) const {
	call(glActiveTexture(GL_TEXTURE0 + slot));
	call(glBindTexture(GL_TEXTURE_2D, m_rendererID));
}

void texture::unbind() const {
	call(glBindTexture(GL_TEXTURE_2D, 0));
}
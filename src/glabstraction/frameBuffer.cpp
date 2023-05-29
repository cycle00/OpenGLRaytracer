#include "frameBuffer.h"

frameBuffer::frameBuffer() : m_rendererID(0), screenTexture(0) {
	call(glGenTextures(1, &screenTexture));
	call(glActiveTexture(GL_TEXTURE0));
	call(glBindTexture(GL_TEXTURE_2D, screenTexture));
	call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, scene::screenWidth, scene::screenHeight, 0, GL_RGBA, GL_FLOAT, NULL));
	call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

	call(glGenFramebuffers(1, &m_rendererID));
	call(glBindFramebuffer(GL_FRAMEBUFFER, m_rendererID));
	call(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0));
}

frameBuffer::~frameBuffer() {
	call(glDeleteFramebuffers(1, &m_rendererID));
}

bool frameBuffer::checkStatus() const {
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		return false;
	}
	return true;
}

void frameBuffer::bind() const {
	call(glBindFramebuffer(GL_FRAMEBUFFER, m_rendererID));
}

void frameBuffer::unbind() const {
	call(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}
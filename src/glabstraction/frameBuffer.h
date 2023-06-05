#pragma once

#include "../scene.h"

#include "../renderer.h"

class frameBuffer {
private:
	unsigned int m_rendererID;
	unsigned int screenTexture;

public:
	frameBuffer();
	~frameBuffer();

	bool checkStatus() const;
	void bind() const;
	void unbind() const;
};
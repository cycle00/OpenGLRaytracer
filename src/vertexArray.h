#pragma once

#include "vertexBuffer.h"

class vertexBufferLayout;

class vertexArray
{
private:
	unsigned int m_rendererID;
public:
	vertexArray();
	~vertexArray();

	void addBuffer(const vertexBuffer& vb, const vertexBufferLayout& layout);
	void bind() const;
	void unbind() const;
};
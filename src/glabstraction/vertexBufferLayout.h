#pragma once

#include <stdexcept>
#include <vector>

#include "../renderer.h"

// defines what a vertex element is
struct vertexBufferElement {
	unsigned int type;
	unsigned int count;
	unsigned char normalized;

	static unsigned int getSizeOfType(unsigned int type) {
		switch (type) {
			case GL_FLOAT: return 4;
			case GL_UNSIGNED_INT: return 4;
			case GL_UNSIGNED_BYTE: return 1;
		}
		assert(false);
		return 0;
	}
};

class vertexBufferLayout
{
private:
	std::vector<vertexBufferElement> m_elements;
	unsigned int m_stride;
public:
	vertexBufferLayout() : m_stride(0) {}

	// all of these push vertex attributes to the elements list
	template<typename T>
	void push(unsigned int count) {
		std::runtime_error(false);
	}

	template<>
	void push<float>(unsigned int count) {
		m_elements.push_back({ GL_FLOAT, count, GL_FALSE });
		m_stride += vertexBufferElement::getSizeOfType(GL_FLOAT) * count;
	}

	template<>
	void push<unsigned int>(unsigned int count) {
		m_elements.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
		m_stride += vertexBufferElement::getSizeOfType(GL_UNSIGNED_INT) * count;
	}

	template<>
	void push<unsigned char>(unsigned int count) {
		m_elements.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
		m_stride += vertexBufferElement::getSizeOfType(GL_UNSIGNED_BYTE) * count;
	}

	inline const std::vector<vertexBufferElement> getElements() const { return m_elements; }
	inline unsigned int getStride() const { return m_stride; }
};
#include "vertexArray.h"

#include "vertexBufferLayout.h"
#include "../renderer.h"

// generate the vao
vertexArray::vertexArray() {
	call(glGenVertexArrays(1, &m_rendererID));
}

// obvious
vertexArray::~vertexArray() {
	call(glDeleteVertexArrays(1, &m_rendererID));
}

// apply vertex attributes to the vao
void vertexArray::addBuffer(const vertexBuffer& vb, const vertexBufferLayout& layout) {
	bind();
	vb.bind();
	const auto& elements = layout.getElements();
	unsigned int offset = 0;
	for (unsigned int i = 0; i < elements.size(); i++) {
		const auto& element = elements[i];
		call(glEnableVertexAttribArray(i));
		call(glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.getStride(), (const void*)offset));
		offset += element.count * vertexBufferElement::getSizeOfType(element.type);
	}
}

// bind the vao
void vertexArray::bind() const {
	call(glBindVertexArray(m_rendererID));
}

// unbind the vao
void vertexArray::unbind() const {
	call(glBindVertexArray(0));
}
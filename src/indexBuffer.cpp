#include "indexBuffer.h"

#include "renderer.h"

indexBuffer::indexBuffer(const unsigned int* data, unsigned int count) : m_count(count) {
    call(glGenBuffers(1, &m_rendererID));
    call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID));
    call(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW));
}

indexBuffer::~indexBuffer() {
    call(glDeleteBuffers(1, &m_rendererID));
}

void indexBuffer::bind() const {
    call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID));
}

void indexBuffer::unbind() const {
    call(glBindBuffer(GL_ARRAY_BUFFER, 0));
}
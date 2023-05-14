#include "vertexBuffer.h"

#include "renderer.h"

vertexBuffer::vertexBuffer(const void* data, unsigned int size) {
    call(glGenBuffers(1, &m_rendererID));
    call(glBindBuffer(GL_ARRAY_BUFFER, m_rendererID));
    call(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}

vertexBuffer::~vertexBuffer() {
    call(glDeleteBuffers(1, &m_rendererID))
}

void vertexBuffer::bind() const {
    call(glBindBuffer(GL_ARRAY_BUFFER, m_rendererID));
}

void vertexBuffer::unbind() const {
    call(glBindBuffer(GL_ARRAY_BUFFER, 0));
}
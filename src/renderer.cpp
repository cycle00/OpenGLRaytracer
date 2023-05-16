#include "renderer.h"

#include <iostream>

// oh boy do i love debugging
void GLClearError() {
    while (glGetError());
}

bool GLLogCall(const char* function, const char* file, int line) {
    if (GLenum error = glGetError()) {
        std::cout << "[OpenGL Error] (" << error << "): " << function << " in " << file << ": " << line << std::endl;
        return false;
    }
    return true;
}

void renderer::clear() const {
    call(glClear(GL_COLOR_BUFFER_BIT));
}

void renderer::draw(const vertexArray& va, const indexBuffer& ib, const shader& shader) const {
    shader.bind();
    va.bind();
    ib.bind();

    call(glDrawElements(GL_TRIANGLES, ib.getCount(), GL_UNSIGNED_INT, nullptr));
}
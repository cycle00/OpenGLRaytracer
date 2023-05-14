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
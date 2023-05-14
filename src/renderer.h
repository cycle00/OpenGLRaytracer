#pragma once

#include <GL/glew.h>

// debugging stuff
#define assert(x) if (!(x)) __debugbreak();
#define call(x) GLClearError();\
x;\
assert(GLLogCall(#x, __FILE__, __LINE__))

void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);
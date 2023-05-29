#pragma once

#include <GL/glew.h>

#include "glabstraction/indexBuffer.h"
#include "glabstraction/shader.h"
#include "glabstraction/vertexArray.h"

// debugging stuff
#define assert(x) if (!(x)) __debugbreak();
#define call(x) GLClearError();\
x;\
assert(GLLogCall(#x, __FILE__, __LINE__))

void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);

class renderer {
public:
	void clear() const;
	void draw(const vertexArray& va, const indexBuffer& ib, const shader& shader) const;
};
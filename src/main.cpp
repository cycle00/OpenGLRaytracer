#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

// debugging stuff
#define assert(x) if (!(x)) __debugbreak();
#define call(x) GLClearError();\
x;\
assert(GLLogCall(#x, __FILE__, __LINE__))

static void GLClearError() {
    while (glGetError());
}

static bool GLLogCall(const char* function, const char* file, int line) {
    if (GLenum error = glGetError()) {
        std::cout << "[OpenGL Error] (" << error << "): " << function << " in " << file << ": " << line << std::endl;
        return false;
    }
    return true;
}
// oh boy do i love debugging

struct shaderProgramSource {
    std::string vertexSource;
    std::string fragmentSource;
};

// Read a shader file and split it up into a vertex and fragment shader
static shaderProgramSource parseShader(const std::string& filepath) {
    std::ifstream stream(filepath);
    
    enum class shaderType {
        NONE = -1,
        VERTEX = 0,
        FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];
    shaderType type = shaderType::NONE;
    while (getline(stream, line)) {
        if (line.find("#shader") != std::string::npos) {
            if (line.find("vertex") != std::string::npos) {
                type = shaderType::VERTEX;
            }
            else if (line.find("fragment") != std::string::npos) {
                type = shaderType::FRAGMENT;
            }
        }
        else {
            ss[(int)type] << line << '\n';
        }
    }

    return { ss[0].str(), ss[1].str() };
}

static unsigned int compileShader(unsigned int type, const std::string& source) {
    // Compile shader
    call(unsigned int id = glCreateShader(type));
    const char* src = source.c_str();
    call(glShaderSource(id, 1, &src, nullptr));
    call(glCompileShader(id));
    
    int result;
    call(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    // Handle errors
    if (!result) {
        int length;
        call(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
        char* message = (char*)alloca(length * sizeof(char)); // virgin malloc vs chad alloca
        call(glGetShaderInfoLog(id, length, &length, message));
        std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
        std::cout << message << std::endl;
        call(glDeleteShader(id));
        return 0;
    }

    return id;
}

static unsigned int createShader(const std::string& vertexShader, const std::string& fragShader) {
    // Initialize an empty shader and compile vertex and fragment shaders
    call(unsigned int program = glCreateProgram());
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragShader);

    // Use the shaders
    call(glAttachShader(program, vs));
    call(glAttachShader(program, fs));
    call(glLinkProgram(program));
    call(glValidateProgram(program));

    call(glDeleteShader(vs));
    call(glDeleteShader(fs));

    return program;
}

int main(void)
{
    GLFWwindow* window;

    // Initialize the library
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(640, 480, "OpenGL Window", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cout << "GLEW failed to initialize" << std::endl;
        return -1;
    }
    
    std::cout << glGetString(GL_VERSION) << std::endl;

    // define an array of vertex positions to draw a triangle
    float vertex_array[] = {
        0.5f, 0.5f,
        0.5f, -0.5f,
        -0.5f, -0.5f,
        -0.5f, 0.5f
    };

    // index buffer to prevent duplicate vertices
    unsigned int indices[] = {
        0, 1, 2, 2, 3, 0
    };

    unsigned int vao;
    call(glGenVertexArrays(1, &vao));
    call(glBindVertexArray(vao));

    // Generate a buffer and bind the vertices to that buffer
    unsigned int buffer;
    call(glGenBuffers(1, &buffer));
    call(glBindBuffer(GL_ARRAY_BUFFER, buffer));
    call(glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(float), vertex_array, GL_STATIC_DRAW));

    // Enable and specify vertex attributes
    call(glEnableVertexAttribArray(0));
    call(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0));

    // Generate and bind index buffer
    unsigned int ibo;
    call(glGenBuffers(1, &ibo));
    call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
    call(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW));

    // Use shader
    shaderProgramSource source = parseShader("res/shaders/basic.shader");
    unsigned int shader = createShader(source.vertexSource, source.fragmentSource);
    call(glUseProgram(shader));

    call(int location = glGetUniformLocation(shader, "u_Color"));
    assert(location != -1);
    
    call(glBindVertexArray(0));
    call(glBindBuffer(GL_ARRAY_BUFFER, 0));
    call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    call(glUseProgram(0));

    float n = 0.0f;
    float incr = 0.05f;
    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window)) {
        // Render here
        call(glClear(GL_COLOR_BUFFER_BIT));

        call(glUseProgram(shader));
        call(glUniform4f(location, n, 0.3f, 0.8f, 1.0f));

        call(glBindBuffer(GL_ARRAY_BUFFER, buffer));

        call(glBindVertexArray(vao));
        call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));

        call(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

        if (n < 0.0f || n > 1.0f)
            incr = -incr;

        n += incr;

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    // Clear shader
    call(glDeleteProgram(shader));

    glfwTerminate();
    return 0;
}
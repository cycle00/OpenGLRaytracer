#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "renderer.h"

#include "vertexArray.h"
#include "vertexBuffer.h"
#include "indexBuffer.h"
#include "shader.h"

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

    {
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

        // initialize a vertex array
        vertexArray va;
        // Generate a buffer and bind the vertices to that buffer
        vertexBuffer vb(vertex_array, 4 * 2 * sizeof(float));

        vertexBufferLayout layout;
        layout.push<float>(2);
        va.addBuffer(vb, layout);

        // Generate and bind index buffer
        indexBuffer ib(indices, 6);

        // Use shader
        shader shader("res/shaders/basic.shader");
        shader.bind();

        // unbind all vaos, vbs, ibos, and shaders
        va.unbind();
        vb.unbind();
        ib.unbind();
        shader.unbind();

        float n = 0.0f;
        float incr = 0.05f;
        // Loop until the user closes the window
        while (!glfwWindowShouldClose(window)) {
            // Render here
            call(glClear(GL_COLOR_BUFFER_BIT));

            shader.bind();
            shader.setUniform4f("u_Color", n, 0.3f, 0.8f, 1.0f);

            va.bind();
            ib.bind();

            call(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

            if (n < 0.0f || n > 1.0f)
                incr = -incr;

            n += incr;

            // Swap front and back buffers
            glfwSwapBuffers(window);

            // Poll for and process events
            glfwPollEvents();
        }
    }

    glfwTerminate();
    return 0;
}
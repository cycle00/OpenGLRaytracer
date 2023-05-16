#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "renderer.h"

#include "indexBuffer.h"
#include "shader.h"
#include "texture.h"
#include "vertexArray.h"
#include "vertexBuffer.h"
#include "vertexBufferLayout.h"

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
        float vertices[] = {
            0.5f, 0.5f, 1.0f, 1.0f,
            0.5f, -0.5f, 1.0f, 0.0f,
            -0.5f, -0.5f, 0.0f, 0.0f,
            -0.5f, 0.5f, 0.0f, 1.0f
        };

        // index buffer to prevent duplicate vertices
        unsigned int indices[] = {
            0, 1, 2, 2, 3, 0
        };

        call(glEnable(GL_BLEND));
        call(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        // initialize a vertex array
        vertexArray va;
        // Generate a buffer and bind the vertices to that buffer
        vertexBuffer vb(vertices, 4 * 4 * sizeof(float));

        vertexBufferLayout layout;
        layout.push<float>(2);
        layout.push<float>(2);
        va.addBuffer(vb, layout);

        // Generate and bind index buffer
        indexBuffer ib(indices, 6);

        // Use shader
        shader shader("res/shaders/basic.shader");
        shader.bind();

        texture texture("res/textures/obamna.png");
        texture.bind();
        shader.setUniform1i("u_texture", 0);

        // unbind all vaos, vbs, ibos, and shaders
        va.unbind();
        vb.unbind();
        ib.unbind();
        shader.unbind();

        renderer renderer;

        // Loop until the user closes the window
        while (!glfwWindowShouldClose(window)) {
            // Render here
            renderer.clear();

            shader.bind();

            renderer.draw(va, ib, shader);

            // Swap front and back buffers
            glfwSwapBuffers(window);

            // Poll for and process events
            glfwPollEvents();
        }
    }

    glfwTerminate();
    return 0;
}
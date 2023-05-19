#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/imgui_impl_glfw.h"
#include "vendor/imgui/imgui_impl_opengl3.h"

#include <iostream>

#include "renderer.h"

#include "indexBuffer.h"
#include "shader.h"
#include "texture.h"
#include "vertexArray.h"
#include "vertexBuffer.h"
#include "vertexBufferLayout.h"

#include "scene.h"

glm::mat4 rotationMatrix(1);

int main(void)
{
    GLFWwindow* window;

    // Initialize the library
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Create a borderless fullscreen mode window and its OpenGL context
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    window = glfwCreateWindow(mode->width, mode->height, "OpenGL Window", monitor, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cout << "GLEW failed to initialize" << std::endl;
        return -1;
    }
    
    std::cout << glGetString(GL_VERSION) << std::endl;

    {
        float viewport[] = {
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,   1.0f, 1.0f,
            -1.0f, 1.0f, 0.0f,  0.0f, 1.0f 
        };

        unsigned int index_buffer[] = {
            0, 1, 2,
            2, 3, 0
        };

        call(glEnable(GL_BLEND));
        call(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        glm::vec3 cameraPos(0.0f, 0.0f, 0.0f);
        float cameraPitch = 0.0f;
        float cameraYaw = 0.0f;
        


        // initialize a vertex array
        vertexArray va;
        // Generate a buffer and bind the vertices to that buffer
        vertexBuffer vb(viewport, 4 * 5 * sizeof(float));

        vertexBufferLayout layout;
        layout.push<float>(3);
        layout.push<float>(2);
        va.addBuffer(vb, layout);

        // Generate and bind index buffer
        indexBuffer ib(index_buffer, 6);

        // Use shader
        shader shader("res/shaders/raytrace.shader");
        shader.bind();
        shader.setUniform1f("u_aspectRatio", (float)mode->width / mode->height);
        shader.setUniform3f("u_cameraPos", cameraPos.x, cameraPos.y, cameraPos.z);
        shader.setUniformMat4f("u_rotationMatrix", rotationMatrix);

        scene::object objects[]{
            {SPHERE, {0.0f, 0.0f, 2.0f}, {1.0f, 1.0f, 1.0f}}
        };
        shader.setUniformObject(objects[0], 0);

        // unbind vao, vb, ibo, and shader
        va.unbind();
        vb.unbind();
        ib.unbind();
        shader.unbind();

        renderer renderer;

        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 130");
        ImGui::StyleColorsDark();

        // Loop until the user closes the window
        while (!glfwWindowShouldClose(window)) {
            // Render here
            renderer.clear();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            shader.bind();

            renderer.draw(va, ib, shader);

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // Swap front and back buffers
            glfwSwapBuffers(window);

            // Poll for and process events
            glfwPollEvents();
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}
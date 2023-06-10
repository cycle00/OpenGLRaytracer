#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/imgui_impl_glfw.h"
#include "vendor/imgui/imgui_impl_opengl3.h"

#include <iostream>

#include "renderer.h"

#include "glabstraction/frameBuffer.h"
#include "glabstraction/indexBuffer.h"
#include "glabstraction/shader.h"
#include "glabstraction/texture.h"
#include "glabstraction/vertexArray.h"
#include "glabstraction/vertexBuffer.h"
#include "glabstraction/vertexBufferLayout.h"

#include "guiManager.h"
#include "scene.h"

bool mouseAbsorbed = false;

glm::mat4 rotationMatrix(1);

glm::vec3 cameraPos(0.0f, 1.0f, -1.0f);
float cameraPitch = 0.0f;
float cameraYaw = 0.0f;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE) {
            if (mouseAbsorbed) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                mouseAbsorbed = false;
            }
            else {
                int screenWidth, screenHeight;
                glfwGetWindowSize(window, &screenWidth, &screenHeight);

                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                glfwSetCursorPos(window, screenWidth / 2.0, screenHeight / 2.0);
                mouseAbsorbed = true;
            }
        }
        else if (key == GLFW_KEY_U) {
            if (guiManager::show) {
                guiManager::hideGUI();
            }
            else {
                guiManager::showGUI();
            }
        }
    }
}

bool handleMovement(GLFWwindow* window, double deltaTime, glm::vec3& cameraPosition, float& cameraPitch, float& cameraYaw, glm::mat4* rotationMatrix) {
    bool moved = false;
    
    int screenWidth, screenHeight;
    glfwGetWindowSize(window, &screenWidth, &screenHeight);

    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    glfwSetCursorPos(window, screenWidth / 2.0, screenHeight / 2.0);

    float xOffset = (float)(mouseX - screenWidth / 2.0);
    float yOffset = (float)(mouseY - screenHeight / 2.0);

    // the mouse breaks when im streaming with remote desktop so i added this
    if (glfwGetKey(window, GLFW_KEY_UP)) {
        yOffset -= 5;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN)) {
        yOffset += 5;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT)) {
        xOffset -= 5;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT)) {
        xOffset += 5;
    }

    if (xOffset != 0.0f || yOffset != 0.0f) moved = true;

    cameraYaw += xOffset * 0.002f;
    cameraPitch += yOffset * 0.002f;

    if (cameraPitch > 1.5707f)
        cameraPitch = 1.5707f;
    if (cameraPitch < -1.5707f)
        cameraPitch = -1.5707f;

    *rotationMatrix = glm::rotate(glm::rotate(glm::mat4(1), cameraPitch, glm::vec3(1, 0, 0)), cameraYaw, glm::vec3(0, 1, 0));

    glm::vec3 forward = glm::vec3(glm::vec4(0, 0, -1, 0) * (*rotationMatrix));
    glm::vec3 up(0, 1, 0);
    glm::vec3 right = glm::cross(forward, up);

    glm::vec3 movementDirection(0);

    if (glfwGetKey(window, GLFW_KEY_W)) {
        movementDirection += forward;
    }
    if (glfwGetKey(window, GLFW_KEY_S)) {
        movementDirection -= forward;
    }
    if (glfwGetKey(window, GLFW_KEY_D)) {
        movementDirection += right;
    }
    if (glfwGetKey(window, GLFW_KEY_A)) {
        movementDirection -= right;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE)) {
        movementDirection += up;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
        movementDirection -= up;
    }

    if (glm::length(movementDirection) > 0.0f) {
        cameraPosition += glm::normalize(movementDirection) * (float)deltaTime;
        moved = true;
    }

    return moved;
}

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
    scene::screenWidth = mode->width;
    scene::screenHeight = mode->height;

    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    window = glfwCreateWindow(scene::screenWidth, scene::screenHeight, "OpenGL Window", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwSetKeyCallback(window, keyCallback);

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

        // skybox whatever
        texture skybox("res/skyboxes/mud_road_puresky_4k.hdr");
        skybox.bind(1);

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

        frameBuffer fb;
        if (!fb.checkStatus()) {
            std::cout << "Framebuffer is not complete!" << std::endl;
            return -1;
        }

        shader.setUniform1i("u_screenTexture", 0);
        shader.setUniform1i("u_skyboxTexture", 1);

        scene::materials.push_back(scene::material({ 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, 0.0f, 0.0f, 5.0f, 0.002f));
        scene::addObject(scene::object(1, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f }, 0));
        scene::addLight(scene::pointLight({ 2.0f, 8.0f, -1.0f }, 2.0f, { 1.0f, 1.0f, 1.0f }, 20.0f, 30.0f));

        scene::currShader = &shader;
        scene::updateObjects();
        scene::updateLights();

        renderer renderer;

        guiManager gui(window);

        call(glViewport(0, 0, scene::screenWidth, scene::screenHeight));
        call(glDisable(GL_DEPTH_TEST));

        double deltaTime = 0.0f;
        int accumulatedPasses = 0;
        bool refresh = false;
        // Loop until the user closes the window
        while (!glfwWindowShouldClose(window)) {
            double preTime = glfwGetTime();
            // Poll for and process events
            glfwPollEvents();
            
            if (mouseAbsorbed) {
                if (handleMovement(window, deltaTime, cameraPos, cameraPitch, cameraYaw, &rotationMatrix)) {
                    refresh = true;
                    shader.setUniform3f("u_cameraPos", cameraPos.x, cameraPos.y, cameraPos.z);
                    shader.setUniformMat4f("u_rotationMatrix", rotationMatrix);
                }
            }
            if (refresh) {
                accumulatedPasses = 0;
                refresh = false;
                shader.setUniform1i("u_accumulatedPasses", accumulatedPasses);
            }
            
            // Render here

            gui.newFrame();

            shader.setUniform1f("u_time", preTime);
            scene::setProperties();

            fb.bind();
            shader.setUniform1i("u_directPass", 0);
            renderer.draw(va, ib, shader);
            accumulatedPasses++;

            fb.unbind();
            shader.setUniform1i("u_directPass", 1);
            shader.setUniform1i("u_accumulatedPasses", accumulatedPasses);
            renderer.draw(va, ib, shader);

            gui.render();
            if (gui.worldModified) refresh = true;

            // Swap front and back buffers
            glfwSwapBuffers(window);

            deltaTime = glfwGetTime() - preTime;
        }
    }

    glfwTerminate();
    return 0;
}
#pragma once

#include <GLFW/glfw3.h>

#include <string>

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/imgui_impl_glfw.h"
#include "vendor/imgui/imgui_impl_opengl3.h"

#include "scene.h"

class guiManager {
private:
	GLFWwindow* window;
	bool showObjectEdit = false;
	bool showLightEdit = false;
	bool showPlaneEdit = false;
	bool showMaterialList = false;
	bool showMaterialEdit = false;
public:
	guiManager(GLFWwindow* window);
	~guiManager();

	static bool show;

	void newFrame();
	void objectEdit();
	void lightEdit();
	void planeEdit();
	void materialList();
	void materialEdit();
	void render();
	static void showGUI();
	static void hideGUI();
};
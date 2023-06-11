#include "guiManager.h"

bool guiManager::show = true;
bool guiManager::worldModified = false;

guiManager::guiManager(GLFWwindow* window) : window(window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.Fonts->AddFontFromFileTTF("res/fonts/Roboto-Medium.ttf", 15.0f);
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
}

guiManager::~guiManager() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void guiManager::newFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void guiManager::objectEdit() {
    // new object
    if (scene::selectedObjectIndex == -1) {
        scene::object o = scene::object();
        scene::addObject(o);
        scene::selectedObjectIndex = scene::objects.size() - 1;
    }

    scene::object prev = scene::objects[scene::selectedObjectIndex];

    ImGui::Begin(std::string("Object ").append(std::to_string(scene::selectedObjectIndex)).c_str(), &showObjectEdit);
    
    ImGui::Text("Type");
    ImGui::RadioButton("None", (int*)&scene::objects[scene::selectedObjectIndex].type, 0); ImGui::SameLine();
    ImGui::RadioButton("Sphere", (int*)&scene::objects[scene::selectedObjectIndex].type, 1); ImGui::SameLine();
    ImGui::RadioButton("Cube", (int*)&scene::objects[scene::selectedObjectIndex].type, 2);

    ImGui::Text("Position");
    ImGui::DragFloat3("##position", scene::objects[scene::selectedObjectIndex].position, 0.005f);

    if (scene::objects[scene::selectedObjectIndex].type != 1) {
        ImGui::Text("Scale");
        ImGui::DragFloat3("##scale", scene::objects[scene::selectedObjectIndex].scale, 0.005f);
    }
    else {
        ImGui::Text("Radius");
        ImGui::DragFloat("##radius", &scene::objects[scene::selectedObjectIndex].scale[0], 0.005f);
    }

    if (ImGui::Button("Material")) {
        showMaterialList = true;
    }
    ImGui::SameLine();
    ImGui::Text(std::string("Material ").append(std::to_string(scene::objects[scene::selectedObjectIndex].mat)).c_str());

    ImGui::Spacing();
    
    if (scene::objects[scene::selectedObjectIndex] != prev) {
        worldModified = true;
    }

    if (ImGui::Button("Delete Object")) {
        showObjectEdit = false;
        scene::removeObject(scene::selectedObjectIndex);
        scene::selectedObjectIndex--;
        worldModified = true;
    }

    ImGui::End();
    scene::updateObjects();
}

void guiManager::materialList() {
    ImGui::Begin("Material List", &showMaterialList);
    ImGui::TextColored(ImVec4(0.8f, 1.0f, 1.0f, 1.0f), "Materials");
    ImGui::SameLine();
    if (ImGui::Button("+##mat")) {
        scene::selectedMaterialIndex = -1;
        showMaterialEdit = true;
        worldModified = true;
    }
    ImGui::BeginChild("Materials", ImVec2(200, 100), true);
    for (unsigned int i = 0; i < scene::materials.size(); i++) {
        if (ImGui::SmallButton(std::string("Material ").append(std::to_string(i)).c_str())) {
            scene::selectedMaterialIndex = i;
            if (scene::planeSelected) scene::planeMaterial = scene::selectedMaterialIndex;
            else scene::objects[scene::selectedObjectIndex].mat = scene::selectedMaterialIndex;
            worldModified = true;
            showMaterialEdit = true;
        }
    }
    ImGui::EndChild();
    ImGui::End();
}

void guiManager::materialEdit() {
    // new material
    if (scene::selectedMaterialIndex == -1) {
        scene::materials.push_back(scene::material());
        scene::selectedMaterialIndex = scene::materials.size() - 1;
        if (scene::planeSelected) scene::planeMaterial = scene::selectedMaterialIndex;
        else scene::objects[scene::selectedObjectIndex].mat = scene::selectedMaterialIndex;
    }

    scene::material prev = scene::materials[scene::selectedMaterialIndex];

    ImGui::Begin(std::string("Material ").append(std::to_string(scene::selectedMaterialIndex)).c_str(), &showMaterialEdit);

    ImGui::ColorPicker3("Albedo", scene::materials[scene::selectedMaterialIndex].albedo);
    ImGui::ColorPicker3("Emission", scene::materials[scene::selectedMaterialIndex].emission);
    ImGui::ColorPicker3("Specular", scene::materials[scene::selectedMaterialIndex].specular);
    ImGui::DragFloat("Emission Strength", &scene::materials[scene::selectedMaterialIndex].emissionStrength);
    ImGui::SliderFloat("Roughness", &scene::materials[scene::selectedMaterialIndex].roughness, 0.0f, 1.0f);
    ImGui::DragFloat("Specular Highlight", &scene::materials[scene::selectedMaterialIndex].specularHighlight, 0.05f);
    ImGui::DragFloat("Specular Exponent", &scene::materials[scene::selectedMaterialIndex].specularExponent, 0.001f);
    ImGui::Checkbox("Transparent", &scene::materials[scene::selectedMaterialIndex].transparent);
    ImGui::InputFloat("Index of Refraction", &scene::materials[scene::selectedMaterialIndex].refractiveIndex);

    if (scene::materials[scene::selectedMaterialIndex] != prev) {
        worldModified = true;
    }

    ImGui::End();
}

void guiManager::lightEdit() {
    // new object
    if (scene::selectedLightIndex == -1) {
        scene::pointLight l = scene::pointLight();
        scene::addLight(l);
        scene::selectedLightIndex = scene::lights.size() - 1;
    }

    scene::pointLight prev = scene::lights[scene::selectedLightIndex];

    ImGui::Begin(std::string("Light ").append(std::to_string(scene::selectedLightIndex)).c_str(), &showLightEdit);

    ImGui::Text("Position");
    ImGui::DragFloat3("##position", scene::lights[scene::selectedLightIndex].position, 0.005f);

    ImGui::Text("Radius");
    ImGui::DragFloat("##radius", &scene::lights[scene::selectedLightIndex].radius, 0.005f);

    ImGui::Text("Color");
    ImGui::ColorPicker3("##color", scene::lights[scene::selectedLightIndex].color);

    ImGui::Text("Power");
    ImGui::DragFloat("##power", &scene::lights[scene::selectedLightIndex].power, 0.005f);

    ImGui::Text("Reach");
    ImGui::DragFloat("##reach", &scene::lights[scene::selectedLightIndex].reach, 0.005f);

    ImGui::Spacing();
    
    if (scene::lights[scene::selectedLightIndex] != prev) {
        worldModified = true;
    }

    if (ImGui::Button("Delete Light")) {
        showLightEdit = false;
        scene::removeLight(scene::selectedLightIndex);
        scene::selectedLightIndex--;
        worldModified = true;
    }

    ImGui::End();
    scene::updateLights();
}

void guiManager::showGUI() {
    show = true;
}

void guiManager::hideGUI() {
    show = false;
}

void guiManager::render() {
    if (show) {
        worldModified = false;
        ImGui::Begin("Ray Tracer");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 1.0f, 1.0f), "Objects");
        ImGui::SameLine();
        if (ImGui::Button("+##obj")) {
            scene::selectedObjectIndex = -1;
            showObjectEdit = true;
            worldModified = true;
        }
        ImGui::BeginChild("Objects", ImVec2(200, 100), true);
        for (unsigned int i = 0; i < scene::objects.size(); i++) {
            if (ImGui::SmallButton(std::string("Object ").append(std::to_string(i)).c_str())) {
                scene::selectedObjectIndex = i;
                scene::selectedMaterialIndex = scene::objects[i].mat;
                scene::planeSelected = false;
                showObjectEdit = true;
            }
        }
        ImGui::EndChild();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.8f, 1.0f), "Lights");
        ImGui::SameLine();
        if (ImGui::Button("+##light")) {
            scene::selectedLightIndex = -1;
            showLightEdit = true;
            worldModified = true;
        }
        ImGui::BeginChild("Lights", ImVec2(200, 100), true);
        for (unsigned int i = 0; i < scene::lights.size(); i++) {
            if (ImGui::SmallButton(std::string("Light ").append(std::to_string(i)).c_str())) {
                scene::selectedLightIndex = i;
                showLightEdit = true;
            }
        }
        ImGui::EndChild();
        
        // plane
        ImGui::Spacing();
        if (ImGui::Checkbox("Plane Visible", &scene::planeVisible)) worldModified = true;
        if (ImGui::Button("Plane Material")) {
            scene::planeSelected = true;
            scene::selectedMaterialIndex = scene::planeMaterial;
            showMaterialList = true;
        }
        ImGui::SameLine();
        ImGui::Text(std::string("Material ").append(std::to_string(scene::planeMaterial)).c_str());

        // shadow resolution and other uniform variables
        ImGui::Spacing();
        if (ImGui::DragInt("Shadow Resolution", &scene::shadowResolution)) worldModified = true;
        if (ImGui::DragInt("Light Bounces", &scene::lightBounces)) worldModified = true;
        if (ImGui::DragFloat("Skybox Gamma", &scene::skyboxGamma)) worldModified = true;
        if (ImGui::DragFloat("Skybox Strength", &scene::skyboxStrength)) worldModified = true;
        ImGui::End();

        // everything else here
        if (showObjectEdit)
            objectEdit();
        if (showLightEdit)
            lightEdit();
        if (showMaterialList)
            materialList();
        if (showMaterialEdit)
            materialEdit();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
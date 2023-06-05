#include "scene.h"

#include "glabstraction/shader.h"

namespace scene {
	std::vector<object> objects;
	std::vector<pointLight> lights;
	std::vector<material> materials;

	shader* currShader;

	int selectedObjectIndex = -1;
	int selectedLightIndex = -1;
	int selectedMaterialIndex = -1;

	// properties
	int screenWidth = 0;
	int screenHeight = 0;
	int shadowResolution = 50;

	material::material() {
		for (int i = 0; i < 3; i++) {
			this->albedo[i] = 1.0f;
			this->emission[i] = 0.0f;
			this->specular[i] = 0.0f;
		}
		this->emissionStrength = 0.0f;
		this->roughness = 1.0f;
	}

	material::material(const std::initializer_list<float>& albedo, const std::initializer_list<float>& emission, const std::initializer_list<float>& specular, float emissionStrength, float roughness) {
		for (int i = 0; i < 3; i++) {
			this->albedo[i] = *(albedo.begin() + i);
			this->emission[i] = *(emission.begin() + i);
			this->specular[i] = *(specular.begin() + i);
		}
		this->emissionStrength = emissionStrength;
		this->roughness = roughness;
	}

	object::object() {
		for (int i = 0; i < 3; i++) {
			this->position[i] = 0.0f;
			this->scale[i] = 1.0f;
		}
		this->type = 0;
		this->mat = material();
	}

	object::object(unsigned int type, const std::initializer_list<float>& position, const std::initializer_list<float>& scale, material mat) {
		for (int i = 0; i < 3; i++) {
			this->position[i] = *(position.begin() + i);
			this->scale[i] = *(scale.begin() + i);
		}
		this->type = type;
		this->mat = mat;
	}

	pointLight::pointLight() {
		for (int i = 0; i < 3; i++) {
			this->position[i] = 0.0f;
			this->color[i] = 1.0f;
		}
		this->radius = 0.0f;
		this->power = 0.0f;
		this->reach = 0.0f;
	}

	pointLight::pointLight(const std::initializer_list<float>& position, float radius, const std::initializer_list<float>& color, float power, float reach) {
		for (int i = 0; i < 3; i++) {
			this->position[i] = *(position.begin() + i);
			this->color[i] = *(color.begin() + i);
		}
		this->radius = radius;
		this->power = power;
		this->reach = reach;
	}

	void setProperties() {
		(*currShader).setUniform1i("u_shadowResolution", shadowResolution);
		// others like light bounces, skybox settings, etc
	}

	void updateObjects() {
		for (unsigned int i = 0; i < objects.size(); i++) {
			(*currShader).setUniformObject(objects[i], i);
		}
	}

	void updateLights() {
		for (unsigned int i = 0; i < lights.size(); i++) {
			(*currShader).setUniformLight(lights[i], i);
		}
	}

	void addObject(object o) {
		objects.push_back(o);
	}

	void removeObject(unsigned int index) {
		objects.erase(objects.begin() + index);
		(*currShader).setUniformObject(object(), index); // erases from world
	}

	void addLight(pointLight l) {
		lights.push_back(l);
	}

	void removeLight(unsigned int index) {
		lights.erase(lights.begin() + index);
		(*currShader).setUniformLight(pointLight(), index); // erases from world
	}
}
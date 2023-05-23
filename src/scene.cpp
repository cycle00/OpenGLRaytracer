#include "scene.h"

#include "shader.h"

namespace scene {
	std::vector<object> objects;
	std::vector<pointLight> lights;
	std::vector<material> materials;

	shader* currShader;

	int selectedObjectIndex = -1;
	int selectedLightIndex = -1;
	int selectedMaterialIndex = -1;

	// properties

	int shadowResolution = 20;

	material::material() {
		for (int i = 0; i < 3; i++) {
			this->albedo[i] = 1.0f;
		}
		this->roughness = 0.0f;
	}

	material::material(const std::initializer_list<float>& albedo, float roughness) {
		for (int i = 0; i < 3; i++) {
			this->albedo[i] = *(albedo.begin());
		}
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
	}

	void addLight(pointLight l) {
		lights.push_back(l);
	}

	void removeLight(unsigned int index) {
		lights.erase(lights.begin() + index);
	}
}
#include "scene.h"

#include <algorithm>

#include "glabstraction/shader.h"

bool compare3f(float* f1, float* f2) {
	return (f1[0] == f2[0] && f1[1] == f2[1] && f1[2] == f2[2]);
}

namespace scene {
	std::vector<object> objects;
	std::vector<pointLight> lights;
	std::vector<material> materials;
	int planeMaterial = 0;

	shader* currShader;

	int selectedObjectIndex = 0;
	bool planeSelected = false;
	int selectedLightIndex = 0;
	int selectedMaterialIndex = 0;

	// properties
	int screenWidth = 0;
	int screenHeight = 0;
	int shadowResolution = 50;
	int lightBounces = 5;
	float skyboxGamma = 2.2f;
	float skyboxStrength = 0.4f;
	bool planeVisible = true;

	material::material() {
		this->id = materials.size();
		for (int i = 0; i < 3; i++) {
			this->albedo[i] = 1.0f;
			this->emission[i] = 0.0f;
			this->specular[i] = 0.0f;
		}
		this->emissionStrength = 0.0f;
		this->roughness = 1.0f;
		this->specularHighlight = 0.0f;
		this->specularExponent = 0.0f;
	}

	material::material(const std::initializer_list<float>& albedo, const std::initializer_list<float>& emission, const std::initializer_list<float>& specular, float emissionStrength, float roughness, float specularHighlight, float specularExponent) {
		this->id = materials.size();
		for (int i = 0; i < 3; i++) {
			this->albedo[i] = *(albedo.begin() + i);
			this->emission[i] = *(emission.begin() + i);
			this->specular[i] = *(specular.begin() + i);
		}
		this->emissionStrength = emissionStrength;
		this->roughness = roughness;
		this->specularHighlight = specularHighlight;
		this->specularExponent = specularExponent;
	}

	bool material::operator==(material m) {
		if (compare3f(this->albedo, m.albedo) &&
			compare3f(this->emission, m.emission) &&
			compare3f(this->specular, m.specular) &&
			this->emissionStrength == m.emissionStrength &&
			this->roughness == m.roughness &&
			this->specularHighlight == m.specularHighlight &&
			this->specularExponent == m.specularExponent) return true;
		else return false;
	}

	bool material::operator!=(material m) {
		if (!compare3f(this->albedo, m.albedo) ||
			!compare3f(this->emission, m.emission) ||
			!compare3f(this->specular, m.specular) ||
			this->emissionStrength != m.emissionStrength ||
			this->roughness != m.roughness ||
			this->specularHighlight != m.specularHighlight ||
			this->specularExponent != m.specularExponent) return true;
		else return false;
	}

	object::object() {
		for (int i = 0; i < 3; i++) {
			this->position[i] = 0.0f;
			this->scale[i] = 1.0f;
		}
		this->type = 0;
		this->mat = 0;
	}

	object::object(unsigned int type, const std::initializer_list<float>& position, const std::initializer_list<float>& scale, int materialID) {
		for (int i = 0; i < 3; i++) {
			this->position[i] = *(position.begin() + i);
			this->scale[i] = *(scale.begin() + i);
		}
		this->type = type;
		this->mat = materialID;
	}

	bool object::operator!=(object o) {
		if (!compare3f(this->position, o.position) ||
			!compare3f(this->scale, o.scale) ||
			this->type != o.type ||
			this->mat != o.mat) return true;
		return false;
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

	bool pointLight::operator!=(pointLight l) {
		if (!compare3f(this->color, l.color) ||
			!compare3f(this->position, l.position) ||
			this->radius != l.radius ||
			this->power != l.power ||
			this->reach != l.reach) return true;
		else return false;
	}

	void setProperties() {
		(*currShader).setUniform1i("u_shadowResolution", shadowResolution);
		(*currShader).setUniform1i("u_lightBounces", lightBounces);
		(*currShader).setUniform1f("u_skyboxGamma", skyboxGamma);
		(*currShader).setUniform1f("u_skyboxStrength", skyboxStrength);
		(*currShader).setUniform1i("u_planeVisible", planeVisible);
		(*currShader).setUniformMaterial("u_planeMaterial", scene::materials[planeMaterial]);
		// other properties
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
#pragma once

#include <initializer_list>
#include <vector>

#define SPHERE 1
#define CUBE 2

class shader;

namespace scene {
	struct material {
		int id;
		float albedo[3];
		float emission[3];
		float specular[3];
		float emissionStrength;
		float roughness;

		material();
		material(const std::initializer_list<float>& albedo, const std::initializer_list<float>& emission, const std::initializer_list<float>& specular, float emissionStrength, float roughness); // add more as needed
		bool operator==(material m);
		bool operator!=(material m);
	};

	struct object {
		unsigned int type;
		float position[3];
		float scale[3];
		material* mat;

		object();
		object(unsigned int type, const std::initializer_list<float>& position, const std::initializer_list<float>& scale, material* mat);
		bool operator!=(object o);
	};

	struct pointLight {
		float position[3];
		float radius;
		float color[3];
		float power;
		float reach;

		pointLight();
		pointLight(const std::initializer_list<float>& position, float radius, const std::initializer_list<float>& color, float power, float reach);
		bool operator!=(pointLight l);
	};

	extern std::vector<object> objects;
	extern std::vector<pointLight> lights;
	extern std::vector<material> materials;

	extern shader* currShader;

	extern int selectedObjectIndex;
	extern int selectedLightIndex;
	extern int selectedMaterialIndex;
	
	// properties
	extern int screenWidth, screenHeight;
	extern int shadowResolution;
	extern int lightBounces;
	extern float skyboxGamma;
	extern float skyboxStrength;

	void updateObjects();
	void updateLights();
	void setProperties();
	void addObject(object o);
	void removeObject(unsigned int index);
	void addLight(pointLight l);
	void removeLight(unsigned int index);
}
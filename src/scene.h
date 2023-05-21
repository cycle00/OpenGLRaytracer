#pragma once

#define SPHERE 1
#define CUBE 2

namespace scene {
	struct material {
		float albedo[3];
		float roughness;

		// constructors
	};

	struct object {
		unsigned int type;
		float position[3];
		float scale[3];

		//object();
		//object(unsigned int type, float position[3], float scale[3]);
	};

	struct pointLight {
		float position[3];
		float radius;
		float color[3];
		float power;
		float reach;

		// constructor
	};
}
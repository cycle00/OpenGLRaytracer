// --------------------------------------------------
#shader vertex
#version 430 core

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec2 vertexUV;

out vec2 fragUV;

void main() {
	fragUV = vertexUV;
	gl_Position = vec4(vertexPos.xyz, 1.0);
}
// --------------------------------------------------
#shader fragment
#version 430 core
// like a lot of this code came from https://github.com/carl-vbn/opengl-raytracing/blob/main/shaders/fragment.glsl, modified to fit my project

#define RENDER_DISTANCE 10000
#define EPSILON 0.0001

in vec2 fragUV;
out vec4 fragColor;

struct Ray {
	vec3 origin;
	vec3 direction;
};

struct SurfacePoint {
	vec3 position;
	vec3 normal;
	// Material material;
};

struct Material {
	vec3 albedo;
};

struct Object {
	uint type;
	vec3 position;
	vec3 scale;
	// Material material;
};

struct PointLight {
	vec3 position;
	float radius;
	vec3 color;
	float power;
	float reach;
};

uniform float u_aspectRatio;
uniform vec3 u_cameraPos;
uniform mat4 u_rotationMatrix;

uniform PointLight u_lights[4];
uniform Object u_objects[64];

bool sphereIntersection(vec3 position, float radius, Ray ray, out float hitDistance) {
	vec3 relativeOrigin = ray.origin - position;

	// ray = a+bt (a = origin, b = direction, t = distance along vector where intersection happens)
	// x^2 + y^2 + z^2 - r^2 = 0; sub in a_x+b_xt for x, a_y+b_yt for y, and a_z+b_zt for z
	// (b_x^2 + b_y^2 + b_z^2)t^2 + (2(a_xb_x + a_yb_y + a_zb_z))t + a_x^2 + a_y^2 + a_z^2 - r^2 = 0

	float a = dot(ray.direction, ray.direction);
	float b = 2.0f * dot(relativeOrigin, ray.direction);
	float c = dot(relativeOrigin, relativeOrigin) - radius * radius;

	float discriminant = b * b - 4.0f * a * c;
	if (discriminant < 0) {
		return false;
	}
	float t1 = (-b - sqrt(discriminant)) / (2 * a);
	if (t1 > 0) {
		hitDistance = t1;
		return true;
	}
	return false;
}

bool planeIntersection(vec3 planeNormal, vec3 planePoint, Ray ray, out float hitDistance) {
	float angle = dot(planeNormal, ray.direction);
	if (abs(angle) > EPSILON) {
		vec3 distance = planePoint - ray.origin;
		hitDistance = dot(distance, planeNormal) / angle;
		return (hitDistance >= EPSILON);
	}
	return false;
}

bool raycast(Ray ray, out SurfacePoint hitPoint) {
	bool didHit = false;
	float minHitDist = RENDER_DISTANCE; // so that no far objects get rendered on top of near objects

	float hitDist;
	for (int i = 0; i < u_objects.length(); i++) {
		if (u_objects[i].type == 0) continue; // object type none
		if (u_objects[i].type == 1 && sphereIntersection(u_objects[i].position, u_objects[i].scale.x, ray, hitDist)) { // sphere
			didHit = true;
			if (hitDist < minHitDist) {
				minHitDist = hitDist;
				hitPoint.position = ray.origin + ray.direction * minHitDist;
				hitPoint.normal = normalize(hitPoint.position - u_objects[i].position);
				// hitPoint.material
			}
		}

		// if (u_objects[i].type == 2)
	}

	if (planeIntersection(vec3(0, 1, 0), vec3(0, 0, 0), ray, hitDist)) {
		didHit = true;
		if (hitDist < minHitDist) {
			minHitDist = hitDist;
			hitPoint.position = ray.origin + ray.direction * minHitDist;
			hitPoint.normal = vec3(0, 1, 0);
			// material
		}
	}

	return didHit;
}

vec3 directIllumination(SurfacePoint hitPoint) {
	vec3 illumination = vec3(0);
	for (int i = 0; i < u_lights.length(); i++) {
		PointLight light = u_lights[i];
		float lightDistance = length(light.position - hitPoint.position);
		if (lightDistance > light.reach) continue;

		// illumination = light_color * object_albedo * cos(angle_between_normal_and_light_direction) -> (dot(normal, light_direction))
		vec3 lightDirection = normalize(light.position - hitPoint.position);
		float diffuse = clamp(dot(hitPoint.normal, lightDirection), 0.0, 1.0);
		SurfacePoint shadowRayHit;
		if (!raycast(Ray(hitPoint.position, lightDirection), shadowRayHit)) {
			float attenuation = lightDistance * lightDistance;
			illumination += light.color * light.power * diffuse * vec3(1.0, 0.0, 1.0) / attenuation;
		}
	}
	return illumination;
}

void main() {
	vec2 centeredUV = (fragUV * 2 - vec2(1)) * vec2(u_aspectRatio, 1.0); // centers the uv so that rays diverge from the center, not a corner and calculates divergence
	vec3 rayDir = (normalize(vec4(centeredUV, -1.0, 0.0)) * u_rotationMatrix).xyz;
	Ray cameraRay = Ray(u_cameraPos, rayDir);

	SurfacePoint hitPoint;
	if (raycast(cameraRay, hitPoint)) {
		fragColor = vec4(directIllumination(hitPoint), 1.0f);
	}
}
// --------------------------------------------------


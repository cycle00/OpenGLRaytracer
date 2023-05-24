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
#define PI 3.1415926538

in vec2 fragUV;
out vec4 fragColor;

struct Ray {
	vec3 origin;
	vec3 direction;
};

struct Material {
	vec3 albedo;
	vec3 emission;
	float emissionStrength;
	float roughness;
};

struct SurfacePoint {
	vec3 position;
	vec3 normal;
	Material material;
};

struct Object {
	uint type;
	vec3 position;
	vec3 scale;
	Material material;
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
uniform float u_time; // seed

uniform int u_shadowResolution;
uniform PointLight u_lights[4];
uniform Object u_objects[64];

// https://thebookofshaders.com/10/
float rand(vec2 seed) {
	return fract(sin(dot(seed, vec2(12.9898, 78.233))) *43758.5453123);
}

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
				hitPoint.material = u_objects[i].material;
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
			hitPoint.material = Material(vec3(1.0, 1.0, 1.0), vec3(0.0, 0.0, 0.0), 0.0f, 1.0);
		}
	}

	return didHit;
}

// Adapted from https://bitbucket.org/Daerst/gpu-ray-tracing-in-unity/src/Tutorial_Pt2/Assets/RayTracingShader.compute
mat3x3 getTangentSpace(vec3 normal)
{
	// Choose a helper vector for the cross product
	vec3 helper = vec3(1, 0, 0);
	if (abs(normal.x) > 0.99)
		helper = vec3(0, 0, 1);

	// Generate vectors
	vec3 tangent = normalize(cross(normal, helper));
	vec3 binormal = normalize(cross(normal, tangent));
	return mat3x3(tangent, binormal, normal);
}

// Adapted from https://bitbucket.org/Daerst/gpu-ray-tracing-in-unity/src/Tutorial_Pt2/Assets/RayTracingShader.compute
vec3 sampleHemisphere(vec3 normal, float alpha, vec2 seed)
{
	// Sample the hemisphere, where alpha determines the kind of the sampling
	float cosTheta = pow(rand(seed), 1.0 / (alpha + 1.0));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	float phi = 2 * PI * rand(seed.yx);
	vec3 tangentSpaceDir = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);

	// Transform direction to world space
	return getTangentSpace(normal) * tangentSpaceDir;
}

vec3 directIllumination(SurfacePoint hitPoint, float seed) {
	vec3 illumination = vec3(0);
	for (int i = 0; i < u_lights.length(); i++) {
		PointLight light = u_lights[i];
		float lightDistance = length(light.position - hitPoint.position);
		if (lightDistance > light.reach) continue;

		// illumination = light_color * object_albedo * cos(angle_between_normal_and_light_direction) -> (dot(normal, light_direction))
		float diffuse = clamp(dot(hitPoint.normal, normalize(light.position - hitPoint.position)), 0.0, 1.0);
		if (diffuse > EPSILON) { // or if roughness is less than one (but that doesnt matter rn)
			// this is basically directly taken from https://github.com/carl-vbn/opengl-raytracing/blob/main/shaders/fragment.glsl because i dont know a better way to find the right amound of shadow rays
			int shadowRays = int(u_shadowResolution * light.radius * light.radius / (lightDistance * lightDistance) + 1);
			int shadowRayHits = 0;
			for (int i = 0; i < shadowRays; i++) {
				vec3 lightSurfacePoint = light.position + normalize(vec3(rand(vec2(i + seed, 1) + hitPoint.position.xy), rand(vec2(i + seed, 2) + hitPoint.position.yz), rand(vec2(i + seed, 3) + hitPoint.position.xz))) * light.radius;
				vec3 lightDirection = normalize(lightSurfacePoint - hitPoint.position);
				vec3 rayOrigin = hitPoint.position + lightDirection * EPSILON * 2.0;
				float maxRayLength = length(lightSurfacePoint - rayOrigin);
				Ray shadowRay = Ray(rayOrigin, lightDirection);
				SurfacePoint shadowRayHit;
				if (raycast(shadowRay, shadowRayHit)) {
					if (length(shadowRayHit.position - rayOrigin) < maxRayLength) {
						shadowRayHits += 1;
					}
				}
			}

			float attenuation = lightDistance * lightDistance;
			illumination += light.color * light.power * diffuse * hitPoint.material.albedo * (1.0-float(shadowRayHits)/shadowRays) / attenuation;
		}
	}
	return illumination;
}

// yeah so glsl prohibits recursion so thats cool
vec3 calculateGI(Ray cameraRay, float seed) {
	vec3 gi = vec3(0);
	vec3 rayOrigin = cameraRay.origin;
	vec3 rayDirection = cameraRay.direction;
	vec3 energy = vec3(1);
	for (int i = 0; i < 50; i++) {
		SurfacePoint hitPoint;
		if (raycast(Ray(rayOrigin, rayDirection), hitPoint)) {
			// emission
			gi += energy * hitPoint.material.emission * hitPoint.material.emissionStrength;

			gi += energy * directIllumination(hitPoint, seed);
			
			// diffuse reflections
			if (hitPoint.material.albedo != vec3(0)) {
				rayOrigin = hitPoint.position + hitPoint.normal * EPSILON;
				rayDirection = sampleHemisphere(hitPoint.normal, 1.0, hitPoint.position.zx + vec2(hitPoint.position.y) + vec2(seed, i));
				energy *= hitPoint.material.albedo * clamp(dot(hitPoint.normal, rayDirection), 0.0, 1.0);
			}
			else {
				break;
			}
		}
	}

	return gi;
}

void main() {
	vec2 centeredUV = (fragUV * 2 - vec2(1)) * vec2(u_aspectRatio, 1.0); // centers the uv so that rays diverge from the center, not a corner and calculates divergence
	vec3 rayDir = (normalize(vec4(centeredUV, -1.0, 0.0)) * u_rotationMatrix).xyz;
	Ray cameraRay = Ray(u_cameraPos, rayDir);

	fragColor = vec4(calculateGI(cameraRay, u_time), 1.0);
}
// --------------------------------------------------


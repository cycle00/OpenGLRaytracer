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
	vec3 specular;
	float emissionStrength;
	float roughness;
	float specularHighlight;
	float specularExponent;
	bool transparent;
	float refractiveIndex;
};

struct SurfacePoint {
	vec3 position;
	vec3 normal;
	Material material;

	bool frontFace;
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
uniform sampler2D u_screenTexture;
uniform sampler2D u_skyboxTexture;
uniform bool u_directPass;
uniform int u_accumulatedPasses;
uniform float u_schlickPass;

uniform int u_shadowResolution;
uniform int u_lightBounces;
uniform float u_skyboxGamma;
uniform float u_skyboxStrength;
uniform bool u_planeVisible;
uniform Material u_planeMaterial;
uniform PointLight u_lights[4];
uniform Object u_objects[64];

// https://thebookofshaders.com/10/
float rand(vec2 seed) {
	return fract(sin(dot(seed, vec2(12.9898, 78.233))) * 43758.5453123);
}

bool sphereIntersection(vec3 position, float radius, Ray ray, out float hitDistance) {
	vec3 relativeOrigin = ray.origin - position;

	// ray = a+bt (a = origin, b = direction, t = distance along vector where intersection happens)
	// x^2 + y^2 + z^2 - r^2 = 0; sub in a_x+b_xt for x, a_y+b_yt for y, and a_z+b_zt for z
	// (b_x^2 + b_y^2 + b_z^2)t^2 + (2(a_xb_x + a_yb_y + a_zb_z))t + a_x^2 + a_y^2 + a_z^2 - r^2 = 0

	float a = dot(ray.direction, ray.direction);
	float half_b = dot(relativeOrigin, ray.direction);
	float c = dot(relativeOrigin, relativeOrigin) - radius * radius;

	float discriminant = half_b * half_b - a * c;
	if (discriminant < 0) {
		return false;
	}
	float sqrtd = sqrt(discriminant);
	float root = (-half_b - sqrtd) / a;
	if (root < 0) {
		root = (-half_b + sqrtd) / a;
		if (root < 0)
			return false;
	}
	hitDistance = root;
	return true;
}

// https://github.com/carl-vbn/opengl-raytracing/blob/main/shaders/fragment.glsl
bool boxIntersection(vec3 position, vec3 scale, Ray ray, out float hitDistance) {
	float t1 = -1000000000000.0;
	float t2 = 1000000000000.0;

	vec3 boxMin = position - scale / 2.0;
	vec3 boxMax = position + scale / 2.0;

	vec3 t0s = (boxMin - ray.origin) / ray.direction;
	vec3 t1s = (boxMax - ray.origin) / ray.direction;

	vec3 tsmaller = min(t0s, t1s);
	vec3 tbigger = max(t0s, t1s);

	t1 = max(t1, max(tsmaller.x, max(tsmaller.y, tsmaller.z)));
	t2 = min(t2, min(tbigger.x, min(tbigger.y, tbigger.z)));
	
	hitDistance = t1;
	if (t1 < 0) hitDistance = t2;
	return ((t1 >= 0 || t2 >= 0) && t1 <= t2);
}

vec3 boxNormal(vec3 cubePosition, vec3 scale, vec3 surfacePosition)
{
	// Source: https://gist.github.com/Shtille/1f98c649abeeb7a18c5a56696546d3cf
	// step(edge,x) : x < edge ? 0 : 1

	vec3 boxMin = cubePosition - scale / 2.0;
	vec3 boxMax = cubePosition + scale / 2.0;

	vec3 center = (boxMax + boxMin) * 0.5;
	vec3 boxSize = (boxMax - boxMin) * 0.5;
	vec3 pc = surfacePosition - center;
	// step(edge,x) : x < edge ? 0 : 1
	vec3 normal = vec3(0.0);
	normal += vec3(sign(pc.x), 0.0, 0.0) * step(abs(abs(pc.x) - boxSize.x), EPSILON);
	normal += vec3(0.0, sign(pc.y), 0.0) * step(abs(abs(pc.y) - boxSize.y), EPSILON);
	normal += vec3(0.0, 0.0, sign(pc.z)) * step(abs(abs(pc.z) - boxSize.z), EPSILON);
	return normalize(normal);
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
				vec3 outwardNormal = normalize(hitPoint.position - u_objects[i].position);
				hitPoint.frontFace = dot(ray.direction, outwardNormal) < 0;
				hitPoint.normal = hitPoint.frontFace ? outwardNormal : -outwardNormal;
				hitPoint.material = u_objects[i].material;
			}
		}

		if (u_objects[i].type == 2 && boxIntersection(u_objects[i].position, u_objects[i].scale, ray, hitDist)) {
			didHit = true;
			if (hitDist < minHitDist) {
				minHitDist = hitDist;
				hitPoint.position = ray.origin + ray.direction * minHitDist;
				vec3 outwardNormal = boxNormal(u_objects[i].position, u_objects[i].scale, ray.origin + ray.direction * minHitDist);
				hitPoint.frontFace = dot(ray.direction, outwardNormal) < 0;
				hitPoint.normal = hitPoint.frontFace ? outwardNormal : -outwardNormal;
				hitPoint.material = u_objects[i].material;
			}
		}
	}

	if (u_planeVisible && planeIntersection(vec3(0, 1, 0), vec3(0, 0, 0), ray, hitDist)) {
		didHit = true;
		if (hitDist < minHitDist) {
			minHitDist = hitDist;
			hitPoint.position = ray.origin + ray.direction * minHitDist;
			hitPoint.normal = vec3(0, 1, 0);
			hitPoint.material = u_planeMaterial;
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

// https://en.wikipedia.org/wiki/UV_mapping
// https://en.wikipedia.org/wiki/Gamma_correction
vec3 sampleSkybox(vec3 dir) {
	if (u_skyboxStrength == 0.0) return vec3(0);
	// V_out = AV_in^gamma
	// u = 0.5 + (arctan2(dir_z, dir_x))/2pi -> but since we are inside the sphere, should be dir_x, dir_z
	// v = 0.5 + (arcsin(dir_y))/pi
	return u_skyboxStrength * pow(texture(u_skyboxTexture, vec2(0.5 + atan(dir.x, dir.z) / (2 * PI), 0.5 + asin(dir.y) / PI)).xyz, vec3(1 / u_skyboxGamma));
}

// https://raytracing.github.io/books/RayTracingInOneWeekend.html
vec3 refract(vec3 rayDir, vec3 surfaceNormal, float etaOverEtaPrime) {
	// sin(theta') = eta/eta' * sin(theta)
	// R' = R'_perpendicular + R'_parallel
	// R'_perpendicular = eta/eta' * (R + cos(theta) * n)
	// R'_parallel = -sqrt(abs(1 - length(R'_perpendicular)^2)) * n
	// as R and n are normalized, cos(theta) = dot(-R, n)
	// R'_perpendicular = eta/eta' * (R + (dot(-R, n) * n)
	float cosTheta = min(dot(-rayDir, surfaceNormal), 1.0);
	vec3 Rperp = etaOverEtaPrime * (rayDir + cosTheta * surfaceNormal);
	vec3 Rpara = -sqrt(abs(1 - Rperp.length() * Rperp.length())) * surfaceNormal;
	return normalize(Rperp + Rpara);
}

// https://raytracing.github.io/books/RayTracingInOneWeekend.html
float reflectance(float cosine, float ref_idx) {
	// Schlick's approximation
	float r0 = (1 - ref_idx) / (1 + ref_idx);
	r0 = r0 * r0;
	return r0 + (1 - r0) * pow((1 - cosine), 5);
}

vec3 directIllumination(SurfacePoint hitPoint, vec3 cameraPos, float seed) {
	vec3 illumination = vec3(0);
	for (int i = 0; i < u_lights.length(); i++) {
		PointLight light = u_lights[i];
		float lightDistance = length(light.position - hitPoint.position);
		if (lightDistance > light.reach) continue;

		// illumination = light_color * object_albedo * cos(angle_between_normal_and_light_direction) -> (dot(normal, light_direction))
		float diffuse = clamp(dot(hitPoint.normal, normalize(light.position - hitPoint.position)), 0.0, 1.0);
		if (diffuse > EPSILON || hitPoint.material.roughness < 1.0) {
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
			illumination += light.color * light.power * diffuse * hitPoint.material.albedo * (1.0 - float(shadowRayHits) / shadowRays) / attenuation;

			// specular highlights
			vec3 lightDir = normalize(hitPoint.position - light.position);
			vec3 reflectedLightDir = reflect(lightDir, hitPoint.normal);
			vec3 cameraDir = normalize(cameraPos - hitPoint.position);
			// https://en.wikipedia.org/wiki/Specular_highlight and basically ripped from https://github.com/carl-vbn/opengl-raytracing/blob/main/shaders/fragment.glsl but I made sure I understood it before using it obviously
			illumination += hitPoint.material.specularHighlight * light.color * light.power / attenuation * pow(max(dot(cameraDir, reflectedLightDir), 0.0), 1.0 / max(hitPoint.material.specularExponent, EPSILON));
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
	for (int i = 0; i < u_lightBounces; i++) {
		SurfacePoint hitPoint;
		if (raycast(Ray(rayOrigin, rayDirection), hitPoint)) {
			// emission
			gi += energy * hitPoint.material.emission * hitPoint.material.emissionStrength;

			// DI
			gi += energy * directIllumination(hitPoint, rayOrigin, seed);

			// II
			if (hitPoint.material.transparent) {
				// refraction (super cool)
				float refractionRatio = hitPoint.frontFace ? (1.0 / hitPoint.material.refractiveIndex) : hitPoint.material.refractiveIndex;
				float cosTheta = min(dot(-rayDirection, hitPoint.normal), 1.0);
				float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

				if (refractionRatio * sinTheta > 1.0 || reflectance(cosTheta, refractionRatio) > u_schlickPass) {
					rayDirection = reflect(rayDirection, hitPoint.normal);
				}
				else {
					rayDirection = refract(rayDirection, hitPoint.normal, refractionRatio);
				}
				rayOrigin = hitPoint.position + rayDirection * EPSILON;
				energy *= hitPoint.material.albedo;
			}
			else {
				// reflection
				float specChance = dot(hitPoint.material.specular, vec3(1.0 / 3.0));
				float diffChance = dot(hitPoint.material.albedo, vec3(1.0 / 3.0));

				float sum = specChance + diffChance;
				specChance /= sum;
				diffChance /= sum;

				float roulette = rand(hitPoint.position.zx + vec2(hitPoint.position.y) + vec2(seed, i));
				// specular reflections
				if (roulette < specChance) {
					float smoothness = 1.0 - hitPoint.material.roughness;
					float alpha = pow(1000.0, smoothness * smoothness);
					if (smoothness == 1.0) {
						rayDirection = reflect(rayDirection, hitPoint.normal);
					}
					else {
						rayDirection = sampleHemisphere(reflect(rayDirection, hitPoint.normal), alpha, hitPoint.position.zx + vec2(hitPoint.position.y) + vec2(seed, i));
					}
					rayOrigin = hitPoint.position + rayDirection * EPSILON;
					float f = (alpha + 2) / (alpha + 1);
					energy *= hitPoint.material.specular * clamp(dot(hitPoint.normal, rayDirection) * f, 0.0, 1.0);
				}
				// diffuse reflections
				else if (diffChance > 0 && roulette < sum) {
					rayOrigin = hitPoint.position + hitPoint.normal * EPSILON;
					rayDirection = sampleHemisphere(hitPoint.normal, 1.0, hitPoint.position.zx + vec2(hitPoint.position.y) + vec2(seed, i));
					energy *= hitPoint.material.albedo * clamp(dot(hitPoint.normal, rayDirection), 0.0, 1.0);
				}
				else {
					break;
				}
			}
		}
		else {
			// skybox
			gi += energy * sampleSkybox(rayDirection);
			break;
		}
	}

	return gi; // debug, should be gi
}

void main() {
	vec2 centeredUV = (fragUV * 2 - vec2(1)) * vec2(u_aspectRatio, 1.0); // centers the uv so that rays diverge from the center, not a corner and calculates divergence

	if (u_directPass) {
		vec3 rayDir = (normalize(vec4(centeredUV, -1.0, 0.0)) * u_rotationMatrix).xyz;
		Ray cameraRay = Ray(u_cameraPos, rayDir);

		fragColor = texture(u_screenTexture, fragUV);
		float passes = float(u_accumulatedPasses);
		fragColor.x /= passes;
		fragColor.y /= passes;
		fragColor.z /= passes;
	}
	else {
		float blur = 0.002f;

		if (u_accumulatedPasses > 0) centeredUV += vec2(rand(vec2(1, u_time) + fragUV.xy) * blur - blur / 2, rand(vec2(2, u_time) + fragUV.xy) * blur - blur / 2);
		vec3 rayDir = (normalize(vec4(centeredUV, -1.0, 0.0)) * u_rotationMatrix).xyz;
		Ray cameraRay = Ray(u_cameraPos, rayDir);

		vec3 color = calculateGI(cameraRay, u_time);
		fragColor = vec4(color, 1.0);

		if (u_accumulatedPasses > 0) {
			fragColor += texture(u_screenTexture, fragUV);
		}
	}
}
// --------------------------------------------------


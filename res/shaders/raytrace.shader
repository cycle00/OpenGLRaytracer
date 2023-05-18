#shader vertex
#version 430 core

layout(location = 0) in vec3 vertexPos;

void main() {
	gl_Position = vec4(vertexPos.xyz, 1.0);
}

#shader fragment
#version 430 core

struct Ray {
	vec3 origin;
	vec3 direction;
};

struct Material {
	vec3 albedo;
};
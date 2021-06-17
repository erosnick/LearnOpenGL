#version 400

in vec3 position;
in vec3 normal;
in vec2 texCoord;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec3 positionData;
layout (location = 2) out vec3 normalData;
layout (location = 3) out vec3 diffuseColorData;

struct Material{
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	vec3 Ke;
	float shininess;
	float reflectionFactor;
	float refractionFactor;
	// relative index of refraction(n1/n2)
	float ior;
	float eta;
	bool hasNormalMap;
};

uniform Material material;

void main() {
	// Store position, normal, and diffuse color in g-buffer
	positionData = position;
	normalData = normal;
	diffuseColorData = material.Kd;
}
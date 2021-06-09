#version 400
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inTangent;
layout (location = 2) in vec3 inBinormal;
layout (location = 3) in vec3 inNormal;
layout (location = 4) in vec2 inTexcoord;

uniform mat4 worldMatrix;
uniform mat4 mvpMatrix;

out vec3 worldNormal;
out vec3 worldPosition;
out vec3 tangentToWorld1;
out vec3 tangentToWorld2;
out vec3 tangentToWorld3;
out vec2 texcoord;

void main() {
	worldPosition = (worldMatrix * vec4(inPosition, 1.0)).xyz;

	worldNormal = normalize(worldMatrix * vec4(inNormal, 0.0)).xyz;
	vec3 worldTangent = (worldMatrix * vec4(inTangent, 0.0)).xyz;
	vec3 worldBinormal = (worldMatrix * vec4(inBinormal, 0.0)).xyz;

	// Transform tangent, binormal, normal from object space to world space
	// tangentToWorld1, tangentToWorld2 and tangentToWorld3 construct the
	// matrix from tangent space to world space
	tangentToWorld1 = vec3(worldTangent.x, worldBinormal.x, worldNormal.x);
	tangentToWorld2 = vec3(worldTangent.y, worldBinormal.y, worldNormal.y);
	tangentToWorld3 = vec3(worldTangent.z, worldBinormal.z, worldNormal.z);

	texcoord = inTexcoord;

	gl_Position = mvpMatrix * vec4(inPosition, 1.0);
}
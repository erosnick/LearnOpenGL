#version 400

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inTangent;
layout (location = 2) in vec3 inBinormal;
layout (location = 3) in vec3 inNormal;
layout (location = 4) in vec2 inTexcoord;

out vec3 position;
out vec3 normal;
out vec2 texCoord;

uniform mat4 worldMatrix;
uniform mat4 normalMatrix;
uniform mat4 mvpMatrix;

void main() {
	position = (worldMatrix * vec4(inPosition, 1.0)).xyz;
	normal = normalize((normalMatrix * vec4(inNormal, 0.0)).xyz);
	texCoord = inTexcoord;
	gl_Position = mvpMatrix * vec4(inPosition, 1.0);
}
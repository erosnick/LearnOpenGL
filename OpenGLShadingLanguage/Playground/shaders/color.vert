#version 400
layout (location = 0) in vec3 inPosition;

uniform mat4 mvpMatrix;

void main() {
	gl_Position = mvpMatrix * vec4(inPosition, 1.0);
}
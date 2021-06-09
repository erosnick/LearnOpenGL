#version 400
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inUV;

out vec3 color;
out vec2 uv;

void main() {
	uv = inUV;
	gl_Position = vec4(inPosition, 1.0);
}
#version 400

in vec2 texcoord;

uniform sampler2D sceneTexture;

uniform float luminanceThreshold;

layout (location = 0) out vec4 fragColor;

// Approximates the brightness of a RGB value
float luminance(vec3 color) {
	return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
}

vec4 extract() {
	vec4 value = texture(sceneTexture, texcoord);

	return value * clamp(luminance(value.rgb) - luminanceThreshold, 0.0, 1.0) * (1.0 / (1.0 - luminanceThreshold));
}

void main() {
	fragColor = extract();
	// fragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
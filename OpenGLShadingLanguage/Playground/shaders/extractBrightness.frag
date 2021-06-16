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
	// return value;
	return value * clamp(luminance(value.rgb) - luminanceThreshold, 0.0, 1.0) * (1.0 / (1.0 - luminanceThreshold));
	// return vec4(vec3(clamp(luminance(value.rgb) - luminanceThreshold, 0.0, 1.0) * (1.0 / (1.0 - luminanceThreshold))), 1.0);
	// if (luminance(value.rgb) > luminanceThreshold) {
	// 	return vec4(vec3(luminance(value.rgb)), 1.0);
	// }

	// return vec4(0.0);
}

void main() {
	fragColor = extract();
	// fragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
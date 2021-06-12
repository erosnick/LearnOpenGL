#version 400

in vec2 texcoord;

uniform sampler2D sceneTexture;

uniform float edgeThreshold;
uniform vec3 edgeColor;
uniform int width;
uniform int height;

layout (location = 0) out vec4 fragColor;

// Approximates the brightness of a RGB value
float luma(vec3 color) {
	return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
}

void main() {
	float dx = 1.0 / float(width);
	float dy = 1.0 / float(height);

	float s00 = luma(texture(sceneTexture, texcoord + vec2(-dx, dy)).rgb);
	float s10 = luma(texture(sceneTexture, texcoord + vec2(-dx, 0.0)).rgb);
	float s20 = luma(texture(sceneTexture, texcoord + vec2(-dx, -dy)).rgb);
	float s01 = luma(texture(sceneTexture, texcoord + vec2(0.0, dy)).rgb);
	float s21 = luma(texture(sceneTexture, texcoord + vec2(0.0, -dy)).rgb);
	float s02 = luma(texture(sceneTexture, texcoord + vec2(dx, dy)).rgb);
	float s12 = luma(texture(sceneTexture, texcoord + vec2(dx, 0.0)).rgb);
	float s22 = luma(texture(sceneTexture, texcoord + vec2(dx, -dy)).rgb);

	float sx = s00 + 2.0 * s10 + s20 - (s02 + 2.0 * s12 + s22);
	float sy = s00 + 2.0 * s01 + s02 - (s20 + 2.0 * s21 + s22);

	float distance = sx * sx + sy * sy;
	
	if (distance > edgeThreshold) {
		fragColor = vec4(edgeColor, 1.0);
	}
	else {
		fragColor = vec4(0.0);
	}

	// fragColor = vec4(1.0, 0.0, 0.0, 1.0);
	// fragColor = vec4(vec3(luma(texture(sceneTexture, texcoord).rgb)), 1.0);
}
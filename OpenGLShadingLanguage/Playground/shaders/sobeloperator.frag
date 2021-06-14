#version 400

in vec2 texcoord;

uniform sampler2D renderTexture;

uniform float edgeThreshold;
uniform vec3 edgeColor;
uniform int width;
uniform int height;

layout (location = 0) out vec4 fragColor;

// Approximates the brightness of a RGB value
float luminance(vec3 color) {
	return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
}

void main() {
	float dx = 1.0 / float(width);
	float dy = 1.0 / float(height);

	ivec2 pix = ivec2(gl_FragCoord.xy);

    float s00 = luminance(texelFetchOffset(renderTexture, pix, 0, ivec2(-1,1)).rgb);
    float s10 = luminance(texelFetchOffset(renderTexture, pix, 0, ivec2(-1,0)).rgb);
    float s20 = luminance(texelFetchOffset(renderTexture, pix, 0, ivec2(-1,-1)).rgb);
    float s01 = luminance(texelFetchOffset(renderTexture, pix, 0, ivec2(0,1)).rgb);
    float s21 = luminance(texelFetchOffset(renderTexture, pix, 0, ivec2(0,-1)).rgb);
    float s02 = luminance(texelFetchOffset(renderTexture, pix, 0, ivec2(1,1)).rgb);
    float s12 = luminance(texelFetchOffset(renderTexture, pix, 0, ivec2(1,0)).rgb);
    float s22 = luminance(texelFetchOffset(renderTexture, pix, 0, ivec2(1,-1)).rgb);

	// float s00 = luminance(texture(renderTexture, texcoord + vec2(-dx, dy)).rgb);
	// float s10 = luminance(texture(renderTexture, texcoord + vec2(-dx, 0.0)).rgb);
	// float s20 = luminance(texture(renderTexture, texcoord + vec2(-dx, -dy)).rgb);
	// float s01 = luminance(texture(renderTexture, texcoord + vec2(0.0, dy)).rgb);
	// float s21 = luminance(texture(renderTexture, texcoord + vec2(0.0, -dy)).rgb);
	// float s02 = luminance(texture(renderTexture, texcoord + vec2(dx, dy)).rgb);
	// float s12 = luminance(texture(renderTexture, texcoord + vec2(dx, 0.0)).rgb);
	// float s22 = luminance(texture(renderTexture, texcoord + vec2(dx, -dy)).rgb);

	float sx = s00 + 2.0 * s10 + s20 - (s02 + 2.0 * s12 + s22);
	float sy = s00 + 2.0 * s01 + s02 - (s20 + 2.0 * s21 + s22);

	float distance = sx * sx + sy * sy;
	
	vec4 edge = vec4(0.0);

	if (distance > edgeThreshold / 4 && distance < edgeThreshold / 2) {
		edge = vec4(0.25, 0.25, 0.25, 1.0);
	}

	if (distance > edgeThreshold / 2 && distance < edgeThreshold) {
		edge = vec4(0.5, 0.5, 0.5, 1.0);
	}
	
	if (distance > edgeThreshold) {
		edge = vec4(edgeColor, 1.0);
	}

	fragColor = edge;

	// fragColor = vec4(gl_FragCoord.x / width, gl_FragCoord.y / height, 0.0, 1.0);
	// fragColor = vec4(vec3(luminance(texture(renderTexture, texcoord).rgb)), 1.0);
	// fragColor = vec4(texelFetchOffset(renderTexture, pix, 0, ivec2(0, 0)));
}
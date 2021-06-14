#version 400

in vec2 texcoord;

uniform sampler2D renderTexture;

uniform int width;
uniform int height;
uniform float pixelOffsets[5] = float[] (0.0, 1.0, 2.0, 3.0, 4.0);
uniform float weights[5];

uniform bool verticalPass = true;

layout (location = 0) out vec4 fragColor;

void main() {
	float dx = 1.0 / float(width);
	float dy = 1.0 / float(height);
	float ds = dx;

	// weights[0] means the weight for the pixel currently being blurred
	vec4 sum = texture(renderTexture, texcoord) * weights[0];

	// * means the pixels will contribute to the finial result
	// + means the pixel currently being blurred
	//      +
	//      *
	//      *
	//      *
	//      *
	// -****+****+
	//	    *
	//      *
	//      *
	//      *
	//      -
	if (verticalPass) {
		ds = dy;

		for (int i = 1; i < 5; i++) {
			sum += texture(renderTexture, texcoord + vec2(0.0, pixelOffsets[i]) * ds) * weights[i];
			sum += texture(renderTexture, texcoord - vec2(0.0, pixelOffsets[i]) * ds) * weights[i];
		}
	}
	else {
		for (int i = 1; i < 5; i++) {
			sum += texture(renderTexture, texcoord + vec2(pixelOffsets[i], 0.0) * ds) * weights[i];
			sum += texture(renderTexture, texcoord - vec2(pixelOffsets[i], 0.0) * ds) * weights[i];
		}
	}

	fragColor = sum;
	// fragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
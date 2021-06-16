#version 400

in vec2 texcoord;

uniform sampler2D bloomTexture;
uniform sampler2D sceneTexture;

layout (location = 0) out vec4 fragColor;

void main() {
	vec4 sceneColor = texture(sceneTexture, texcoord);
	vec4 bloom = texture(bloomTexture,texcoord);
	fragColor = sceneColor + bloom;
	// fragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
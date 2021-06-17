#version 400

struct Light{
	vec4 color;
	vec4 position;
	vec3 direction;
	float exponent;
	float cutoff;
	float outerCutoff;
	float intensity;
	float Kc;
	float Kl;
	float Kq;
	int type;
};

uniform Light light;

layout (location = 0) out vec4 fragColor;

in vec2 texcoord;

uniform sampler2D positionTexture;
uniform sampler2D normalTexture;
uniform sampler2D diffuseColorTexture;

void main() {
	vec3 normal = texture(normalTexture, texcoord).xyz;
	vec3 position = texture(positionTexture, texcoord).xyz;
	vec3 diffuseColor = texture(diffuseColorTexture, texcoord).xyz;

	vec3 lightDirection = normalize(-light.position.xyz);

	vec3 diffuse = max(0.0, dot(normal, lightDirection)) * diffuseColor;

	fragColor = vec4(diffuse, 1.0);
	// fragColor = vec4(vec3(texcoord.x, texcoord.y, 0.0), 1.0);
}
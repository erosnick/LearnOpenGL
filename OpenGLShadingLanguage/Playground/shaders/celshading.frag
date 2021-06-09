#version 400

in vec3 worldNormal;
in vec3 worldPosition;

layout (location = 0) out vec4 fragColor;

struct Light{
	vec4 color;
	vec4 position;
	vec3 direction;
    float exponent;
    float cutoff;
	float intensity;
	int type;
};

struct Material{
	float Ka;
	float Kd;
	float Ks;
	float shininess;
};

uniform Light lights[3];

uniform Material material;

uniform vec3 ambient = vec3(0.2, 0.2, 0.2);

uniform vec3 eye;

const int levels = 4;

const float scaleFactor = 1.0 / levels;

vec3 blinnPhong(Light light, vec3 worldPosition, vec3 normal, vec3 eye, vec3 albedo, Material material) {
	vec3 lightDirection  = vec3(0.0);
	float attenuation = 1.0;

	if (light.position.w == 0.0) {
		lightDirection = normalize(-light.position.xyz);
	}
	else if (light.position.w >= 1.0) {
		lightDirection = normalize(light.position.xyz - worldPosition);
		attenuation = 1.0 / length(worldPosition - light.position.xyz);
	}

	float nDotL = dot(normal, lightDirection);
	 
	// Standard Lambert's law
	// float diffuse = max(0.0f, nDotL);
	
	float cosine = max(0.0, nDotL);

	// Half Lambert
	vec3 diffuse = (0.5 * nDotL + 0.5) * albedo * material.Kd * floor(cosine * levels) * scaleFactor;
	
	vec3 reflected = reflect(-lightDirection, normal);

	vec3 viewDirection = normalize(eye - worldPosition);
	
	vec3 halfVector = normalize(lightDirection + viewDirection);
	
	float specular = 0.0;

	if(nDotL > 0.0) {
		specular = pow(max(0.0, dot(halfVector, normal)), material.shininess) * material.Ks;
	}

	// Spot Light
	if (light.position.w == 2.0) {
		vec3 spotLightDirection = normalize(light.direction);
		float angle = acos(dot(-lightDirection, spotLightDirection));
		float cutoff = radians(clamp(light.cutoff, 0.0, 90.0));
		// In Spot Light cone
		if (angle < cutoff) {
			attenuation = pow(dot(-lightDirection, spotLightDirection), light.exponent);
		}
		else {
			attenuation = 0.0;
		}
	}

	return (diffuse + specular) * light.color.rgb * light.intensity * attenuation;
	// return vec3(attenuation);
}

void main() {
	vec3 normal = normalize(worldNormal);

	if (!gl_FrontFacing) {
		normal = -normal;
	}

	vec3 light1 = blinnPhong(lights[0], worldPosition, normal, eye, vec3(1.0), material);

	vec3 light2 = blinnPhong(lights[1], worldPosition, normal, eye, vec3(1.0), material);

	vec3 light3 = blinnPhong(lights[2], worldPosition, normal, eye, vec3(1.0), material);

	// fragColor = vec4(light1 + light2 + light3, 1.0);
	fragColor = vec4(light2, 1.0);
}
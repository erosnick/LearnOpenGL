#version 400

in vec3 worldNormal;
in vec3 worldPosition;
in vec3 tangentToWorld1;
in vec3 tangentToWorld2;
in vec3 tangentToWorld3;
in vec2 texcoord;
in vec3 refectionDirection;
in vec3 worldViewDirection;

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
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	float shininess;
};

struct Fog {
	float minDistance;
	float maxDistance;
	float density;
	vec4 color;
};

uniform Light lights[3];

uniform Material material;

uniform Fog fog;

uniform vec3 ambient = vec3(0.2, 0.2, 0.2);

uniform vec3 eye;

uniform samplerCube cubeMap;

uniform sampler2D textures[2];

uniform bool drawSkyBox = false;

uniform float cubeMapReflection = 0.0;

float computeLinearFog(Fog fog, float distance) {
	return clamp((fog.maxDistance - distance) / (fog.maxDistance - fog.minDistance), 0.0, 1.0);
}

float computeExponentFog(Fog fog, float distance, float power) {
	return exp(-pow(fog.density * distance, power));
}

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
	vec3 diffuse = max(0.0, nDotL) * albedo * material.Kd;

	// Half Lambert
	// vec3 diffuse = (0.5 * nDotL + 0.5) * albedo * material.Kd;
	
	vec3 reflected = reflect(-lightDirection, normal);

	vec3 viewDirection = normalize(eye - worldPosition);
	
	vec3 halfVector = normalize(lightDirection + worldViewDirection);
	
	vec3 specular = vec3(0.0);

	if(nDotL > 0.0) {
		specular = pow(max(0.0, dot(halfVector, normal)), material.shininess) * albedo * material.Ks;
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
	// return specular;
}

void main() {

	vec4 albedo = texture(textures[0], texcoord);

	vec3 normal = texture(textures[1], texcoord).xyz;

	normal = normal * 2.0 - 1.0;

	normal = normalize(vec3(dot(tangentToWorld1, normal), dot(tangentToWorld2, normal), dot(tangentToWorld3, normal)));

	// vec3 normal = normalize(worldNormal);

	vec3 ambient = albedo.rgb * material.Ka;

	if (!gl_FrontFacing) {
		normal = -normal;
	}

	vec3 light1 = blinnPhong(lights[0], worldPosition, normal, eye, albedo.rgb, material);

	vec3 light2 = blinnPhong(lights[1], worldPosition, normal, eye, albedo.rgb, material);

	vec3 light3 = blinnPhong(lights[2], worldPosition, normal, eye, albedo.rgb, material);

	float distance = length(eye - worldPosition);

	float fogFactor = computeLinearFog(fog, distance);

	fogFactor = computeExponentFog(fog, distance, 2.0);

	vec3 finalColor = mix(fog.color.rgb, light2 + ambient, fogFactor);

	// fragColor = vec4(light1 + light2 + light3, 1.0);
	vec4 cubeMapColor = texture(cubeMap, refectionDirection);

	if (drawSkyBox) {
		fragColor = cubeMapColor;
	}
	else {
		fragColor = mix(vec4(finalColor, 1.0), cubeMapColor, cubeMapReflection);
		// fragColor = vec4(finalColor, 1.0);
	}
}
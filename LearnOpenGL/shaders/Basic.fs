#version 330 core

in vec3 FragPosition;
in vec3 Normal;
out vec4 fragColor;

uniform float ambientStrength;
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPosition;

void main()
{
    vec3 normal = normalize(Normal);
    vec3 lightDirection = normalize(lightPosition - FragPosition);

    float diffuse = max(dot(normal, lightDirection), 0);
    vec3 diffuseColor = diffuse * lightColor;

    vec3 ambientColor = lightColor * ambientStrength;
    vec3 finalColor = objectColor * (ambientColor + diffuseColor);

    fragColor = vec4(finalColor, 1.0f);
} 
#version 330 core

out vec4 fragColor;

in vec2 texCoord;

uniform sampler2D anotherTexture;
uniform sampler2D ourTexture;
uniform float alpha;

void main()
{
    fragColor = mix(texture(ourTexture, texCoord), texture(anotherTexture, texCoord), alpha);
}
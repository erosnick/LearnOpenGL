#version 330 core

out vec4 fragColor;
in vec3 ourColor;
in vec3 position;

void main()
{
    fragColor = vec4(position, 1.0f);
} 
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 Normal;
out vec3 FragPosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 modelView;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    FragPosition = vec3(model * vec4(aPos, 1.0));
    Normal = vec3(model * vec4(aNormal, 0.0));
}
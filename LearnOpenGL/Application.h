#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Shader.h"
#include "Texture.h"

class Application
{
public:

	Application(int inWidth, int inHeight);

	void run();

	void render();

private:

	void initialize();

	GLFWwindow* createWindow(int Width, int Height);

	bool loadGLLoader();

	void initVBO();

	void prepareResources();

	static void resizeBuffer(GLFWwindow* window, int width, int height);

	void processInput(GLFWwindow* window);

	void internalKeyCallback(GLFWwindow* window, int key, int scanCode, int action, int mods);
	void internalMouseCallback(GLFWwindow* window, double xPos, double yPos);
	void internalScrollCallback(GLFWwindow* window, double xOffset, double yOffset);

	void CheckMaximumVertexAttributesSupport();

private:
	GLFWwindow* appWindow;
	int width;
	int height;
	unsigned int VAO[4];
	GLenum polygonMode = GL_FILL;
	Shader shader;
	Texture texture;
	Texture anotherTexture;
	float aspectRatio;
	float fov = 45.0f;
	float alpha;
	float rotateAngle;

	glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	float cameraSpeed = 1.5f;
	float sensitivity = 0.01f;

	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;

	float lastFrame;
	float deltaTime;

	float pitch;
	float yaw = -90.0f; // yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.

	float lastX;
	float lastY;
	bool firstMouse;
};
#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "Model.h"

class Application
{
public:

	Application(int inWidth, int inHeight);

	void run();

	void render();

private:

	void initialize();

	GLFWwindow* createWindow(int Width, int Height);
	
	void bindCallbacks();

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
	unsigned int VAO[5];
	GLenum polygonMode = GL_FILL;
	Shader textureShader;
	Shader basicShader;
	Shader lightShader;
	Texture texture;
	Texture anotherTexture;

	float aspectRatio;
	float fov = 45.0f;

	float alpha;
	float rotateAngle;

	float sensitivity = 0.01f;

	glm::mat4 modelMatrix;

	float lastFrame;
	float deltaTime;

	float lastX;
	float lastY;
	bool firstMouse;

	Camera camera;
	Model cubeModel;
	Model lightModel;
};
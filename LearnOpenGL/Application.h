#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "Model.h"
#include "imgui/imgui.h"

class Application
{
public:

	Application(int inWidth, int inHeight);

	void run();

	void renderImGui();

	void update(float delta);

	void render();

private:

	void initImGui();

	void buildImGuiWidgets();

	void initialize();

	GLFWwindow* createWindow(int Width, int Height);
	
	void bindCallbacks();

	bool loadGLLoader();

	void initVBO();

	void prepareResources();

	void processInput(GLFWwindow* window);

	void internalResizeBufferCallback(GLFWwindow* window, int inWidth, int inHeight);
	void internalKeyCallback(GLFWwindow* window, int key, int scanCode, int action, int mods);
	void internalMouseMoveCallback(GLFWwindow* window, double xPos, double yPos);
	void internalMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
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

	bool showDemoWindow = true;
	bool showAnotherWindow = false;

	ImVec4 clearColor = ImVec4(0.392f, 0.584f, 0.929f, 1.0f);
	ImVec4 lightColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	ImVec4 lightPosition = ImVec4(1.0f, 0.0f, 0.0f, 0.0f);
	float ambientStrength = 1.0f;

	float aspectRatio;
	float fov = 45.0f;

	float alpha;
	float rotateAngle;

	float timeSlice = 0.01666667f;

	double lastFrame;
	double deltaTime;

	double simulationTime = 0.0f;

	double lastX;
	double lastY;

	bool cameraControl;
	bool cameraPan;
	float panMoveRate = 0.1f;

	double xOffset;
	double yOffset;

	glm::mat4 worldMatrix;

	Camera camera;
	Model cubeModel;
	Model lightModel;
	Model ray;

	std::vector<ImFont*> fonts;
};
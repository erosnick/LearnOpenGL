#include "Application.h"
#include <iostream>
#include <algorithm>
#include <functional>
#include <vector>

Application::Application(int inWidth, int inHeight)
{
	width = inWidth;
	height = inHeight;

	aspectRatio = (float)width / (float)height;

	alpha = 0.5f;

	rotateAngle = -50.0f;

	model = glm::mat4(1.0f);

	lastX = width / 2.0f;
	lastY = height / 2.0f;

	firstMouse = true;

	camera.setFOV(45.0f);
	camera.setAspectRatio(aspectRatio);
	camera.lookAt(glm::vec3(0.0f, 0.0f, 0.0f));

	initialize();

	appWindow = createWindow(width, height);

	if (appWindow == nullptr)
	{
		std::cout << "Failed to create GLFW window." << std::endl;
		glfwTerminate();
	}

	glfwSetInputMode(appWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(appWindow);

	if (!loadGLLoader())
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
	}

	CheckMaximumVertexAttributesSupport();

	bindCallbacks();

	initVBO();

	prepareResources();
}

void Application::run()
{
	while (!glfwWindowShouldClose(appWindow))
	{
		float currentFrame = glfwGetTime();

		deltaTime = currentFrame - lastFrame;

		lastFrame = currentFrame;

		processInput(appWindow);

		render();

		glfwSwapBuffers(appWindow);
		glfwPollEvents();
	}

	glfwTerminate();
}

void Application::render()
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.392f, 0.584f, 0.929f, 1.0f);
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//float timeValue = glfwGetTime();
	//float greenValue = (sin(timeValue) / 2.0f) + 0.5f;

	//shader.use();

	// 这里的glBindTexture可以用来切换不同的texture。
	//texture.use();
	//anotherTexture.use();

	shader.setFloat("alpha", alpha);

	model = glm::mat4(1.0f);

	//rotateAngle += 0.01f;

	model = glm::rotate(model, glm::radians(rotateAngle), glm::vec3(1.0f, 0.0f, 0.0f));

	float radius = 5.0f;

	float cameraX = sin(glfwGetTime()) * radius;      

	shader.setMat4("model", model);
	shader.setMat4("view", camera.viewMatrix());
	shader.setMat4("projection", camera.projectionMatrix());

	glBindVertexArray(VAO[2]);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	//glDrawArrays(GL_POINTS, 0, 36);
	glBindVertexArray(0);
}

void Application::initialize()
{
	// Initialize GLFW. 
	// Use OpenGL3.3, Core-Profile.
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

GLFWwindow* Application::createWindow(int Width, int Height)
{
	GLFWwindow* window = glfwCreateWindow(Width, Height, "LearnOpenGL", nullptr, nullptr);

	return window;
}

void Application::bindCallbacks()
{
	glfwSetFramebufferSizeCallback(appWindow, resizeBuffer);

	glfwSetWindowUserPointer(appWindow, this);

	auto keyCallback = [](GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		static_cast<Application*>(glfwGetWindowUserPointer(window))->internalKeyCallback(window, key, scancode, action, mods);
	};

	glfwSetKeyCallback(appWindow, keyCallback);

	auto mouseCallback = [](GLFWwindow* window, double xpos, double ypos)
	{
		static_cast<Application*>(glfwGetWindowUserPointer(window))->internalMouseCallback(window, xpos, ypos);
	};

	glfwSetCursorPosCallback(appWindow, mouseCallback);

	auto scrollCallback = [](GLFWwindow* window, double xOffset, double yOffset)
	{
		static_cast<Application*>(glfwGetWindowUserPointer(window))->internalScrollCallback(window, xOffset, yOffset);
	};

	glfwSetScrollCallback(appWindow, scrollCallback);
}

bool Application::loadGLLoader()
{
	return gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
}

void Application::initVBO()
{
	// 三角形
	float vertices1[] = {
		 0.0f, -0.5f, 0.0f,
		 0.5f,  0.5f, 0.5f,
		-0.5f,  0.5f, 0.0f
	};

	unsigned int indices1[] = {
		0, 1, 2
	};

	// 三角形-顶点颜色
	float vertices2[] = {
		-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
		 0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f
	};

	unsigned int indices2[] = {
		0, 1, 2
	};

	// 三角形-纹理
	std::vector<float> vertices = {
		0.5f,  0.5f, 0.0f, 1.0f, 1.0f,  // 右上角
		0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  // 右下角
	   -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,  // 左下角
	   -0.5f,  0.5f, 0.0f, 0.0f, 1.0f   // 左上角
	};

	struct Vertex
	{
		Vertex(float inX, float inY, float inZ, float inU, float inV)
		{
			x = inX;
			y = inY;
			z = inZ;
			u = inU;
			v = inV;
		}

		float x;
		float y;
		float z;
		float u;
		float v;
	};

	unsigned int indices[] = { // 注意索引从0开始! 
		0, 1, 3, // 第一个三角形
		1, 2, 3  // 第二个三角形
	};

	std::vector<Vertex> cube = {
			   Vertex(-0.5f,  0.5f,  0.5f, 0.0f, 1.0f),   //正面左上0
			   Vertex(-0.5f, -0.5f,  0.5f, 0.0f, 0.0f),   //正面左下1
			   Vertex( 0.5f, -0.5f,  0.5f, 1.0f, 0.0f),   //正面右下2
			   Vertex( 0.5f,  0.5f,  0.5f, 1.0f, 1.0f),   //正面右上3
			   Vertex(-0.5f,  0.5f, -0.5f, 0.0f, 1.0f),   //反面左上4
			   Vertex(-0.5f, -0.5f, -0.5f, 0.0f, 0.0f),   //反面左下5
			   Vertex( 0.5f, -0.5f, -0.5f, 1.0f, 0.0f),   //反面右下6
			   Vertex( 0.5f,  0.5f, -0.5f, 1.0f, 1.0f)    //反面右上7
	};

	//cube.clear();

	for (int i = 0; i < 100; i++)
	{

	}

	unsigned int cubeIndices[] = {
				0, 3, 2, 0, 2, 1,    //正面
				0, 1, 5, 0, 5, 4,    //左面
				0, 7, 3, 0, 4, 7,    //上面
				6, 7, 4, 6, 4, 5,    //后面
				6, 3, 7, 6, 2, 3,    //右面
				6, 5, 1, 6, 1, 2     //下面
	};

	unsigned int VBO[4];
	unsigned int IBO[4];

	glGenBuffers(4, VBO);
	glGenBuffers(4, IBO);

	glGenVertexArrays(4, VAO);

	// Initialize first VAO
	glBindVertexArray(VAO[0]);

	// Bind vertex data.
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);

	// Bind index data.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices1), indices1, GL_STATIC_DRAW);

	// Location attribute.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)0);
	glEnableVertexAttribArray(0);

	// Color attribute.
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(sizeof(float) * 3));
	glEnableVertexAttribArray(1);

	// Initialize second VAO
	glBindVertexArray(VAO[1]);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices2), indices2, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
	glEnableVertexAttribArray(0);

	// Initialize third VAO
	glBindVertexArray(VAO[2]);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0);
	glEnableVertexAttribArray(0);

	// TexCoord attribute.
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));
	glEnableVertexAttribArray(1);

	// Initialize fourth VAO
	glBindVertexArray(VAO[3]);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);

	glBufferData(GL_ARRAY_BUFFER, cube.size() * sizeof(Vertex), cube.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

	// Position attribute.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0);
	glEnableVertexAttribArray(0);

	// TexCoord attribute.
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));
	glEnableVertexAttribArray(1);
}

void Application::prepareResources()
{
	shader.load("shaders/Texture.vs", "shaders/Texture.fs");

	// don't forget to activate/use the shader before setting uniforms!
	shader.use();

	texture.load("resources/rexie01.jpg", GL_TEXTURE0);

	shader.setInt("ourTexture", 0);

	anotherTexture.load("resources/rexie02.jpg", GL_TEXTURE1);

	shader.setInt("anotherTexture", 1);
}

void Application::resizeBuffer(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void Application::processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		camera.forward(deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		camera.forward(-deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		camera.right(-deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		camera.right(deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		camera.up(deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		camera.up(-deltaTime);
	}
}

void Application::CheckMaximumVertexAttributesSupport()
{
	int numAttributes;

	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &numAttributes);

	cout << "Maximum number of vertex attributes supported: " << numAttributes << endl;
}

void Application::internalKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
	{
		polygonMode = GL_FILL;
	}

	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
	{
		polygonMode = GL_LINE;
	}

	if (key == GLFW_KEY_3 && action == GLFW_PRESS)
	{
		polygonMode = GL_POINT;
	}

	glPolygonMode(GL_FRONT_AND_BACK, polygonMode);

	if (key == GLFW_KEY_UP && action == GLFW_PRESS)
	{
		alpha += 0.1f;

		alpha = std::min(1.0f, alpha);
	}

	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
	{
		alpha -= 0.1f;
		alpha = std::max(0.0f, alpha);
	}

	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
	{
		rotateAngle -= 5.0f;
		rotateAngle = std::max(0.0f, rotateAngle);
	}

	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
	{
		rotateAngle += 5.0f;
		rotateAngle = std::min(360.0f, rotateAngle);
	}
}

void Application::internalMouseCallback(GLFWwindow* window, double xPos, double yPos)
{
	if (firstMouse)
	{
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}

	float xOffset = xPos - lastX;
	float yOffset = lastY - yPos; // reversed since y - coordinates go from bottom to top
	
	lastX = xPos;
	lastY = yPos;

	xOffset *= sensitivity;
	yOffset *= sensitivity;

	float yaw = camera.yaw;
	float pitch = camera.pitch;

	yaw += xOffset;
	pitch += yOffset;

	camera.setYaw(yaw);
	camera.setPitch(pitch);
}

void Application::internalScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
	if (fov >= 1.0f && fov <= 90.0f)
	{
		fov -= yOffset;
	}

	if (fov <= 1.0f)
	{
		fov = 1.0f;
	}

	if (fov >= 90.0f)
	{
		fov = 90.0f;
	}
}
